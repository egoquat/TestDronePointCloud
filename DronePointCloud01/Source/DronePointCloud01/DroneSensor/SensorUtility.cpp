#include "SensorUtility.h"

#include "HighResScreenshot.h"
#include "ImagePixelData.h"
#include "ImageWriteQueue.h"
#include "ImageWriteTask.h"
#include "Engine/Texture2D.h"
#include "Components/PrimitiveComponent.h"
#include "Components/LineBatchComponent.h"
#include "Misc/FeedbackContext.h"
#include "Engine/TextureRenderTarget2D.h"

TMap<int32, FSensorUtility::FProfiler> FSensorUtility::FProfiler::Profilers;
int32 FSensorUtility::FProfiler::CountingRay = 0;
int32 FSensorUtility::FProfiler::CountingRaycast = 0;

FReadSurfaceDataFlags SensorUtilityReadPixelFlags(ERangeCompressionMode::RCM_UNorm);

UTexture2D* FSensorUtility::LoadTexture(const FString& pathTexture)
{
    if (pathTexture.IsEmpty()) return nullptr;
    UObject* loadObject = StaticLoadObject(UTexture2D::StaticClass(), nullptr, *(pathTexture));
    if (loadObject == nullptr ) return nullptr;
    UTexture2D* textureLoad = Cast<UTexture2D>(loadObject);
    ensure(textureLoad);
    return textureLoad;
}

void FSensorUtility::SaveTextureToDiskPixels(const FIntPoint& res, const TArray<FColor>& bitmap, const FString& FilePath)
{
    TUniquePtr<TImagePixelData<FColor>> pixelData = MakeUnique<TImagePixelData<FColor>>(res);
    pixelData->Pixels                             = bitmap;
    
    TUniquePtr<FImageWriteTask> ImageTask = MakeUnique<FImageWriteTask>();
    ImageTask->PixelData = MoveTemp(pixelData);
    ImageTask->Filename = FilePath;
    ImageTask->Format = EImageFormat::PNG;
    ImageTask->CompressionQuality = (int32) EImageCompressionQuality::Default;
    ImageTask->bOverwriteFile = true;
    ImageTask->PixelPreProcessors.Add(TAsyncAlphaWrite<FColor>(255));
    ImageTask->OnCompleted = [=](bool saved) { GWarn->Logf(TEXT("SaveTextureToDisk/%s"), *FilePath); };

    FHighResScreenshotConfig &HighResScreenshotConfig = GetHighResScreenshotConfig();
    HighResScreenshotConfig.ImageWriteQueue->Enqueue(MoveTemp(ImageTask));
}

void FSensorUtility::SavePixelsToDiskDepthTest( const FIntPoint& res,
                                                const TArray<FLinearColor>& bitmapDepth,
                                                const FString& FilePath)
{
    TArray<FColor> bitmap;
    bitmap.AddDefaulted(bitmapDepth.Num());
    for (int i = 0; i < bitmap.Num(); ++i)
    {
        float colorDepth = bitmapDepth[i].A * 1000; 
        bitmap[i].R = colorDepth * 128.0f;
    }
    SaveTextureToDiskPixels(res, bitmap, FilePath);
}

void FSensorUtility::SaveTextureToDisk(UTextureRenderTarget2D* renderTargetTex, const FString& FilePath)
{
    TArray<FColor> bitmap;
    FTextureRenderTargetResource* renderTargetResource = renderTargetTex->GameThread_GetRenderTargetResource();
    if (renderTargetResource == nullptr)
    {
        GWarn->Logf(TEXT("SaveTextureToDisk/(renderTargetResource == nullptr)/%s"), *FilePath);
        return;
    }
    
    renderTargetResource->ReadPixels(bitmap, SensorUtilityReadPixelFlags);

    FIntPoint res(renderTargetTex->SizeX, renderTargetTex->SizeY);
    SaveTextureToDiskPixels(res, bitmap, FilePath);
}

bool FSensorUtility::ReadPixels(UTextureRenderTarget2D* renderTargetTex, TArray<FColor>& bitmapOut)
{
    FTextureRenderTargetResource* renderTargetResource = renderTargetTex->GameThread_GetRenderTargetResource();
    if (renderTargetResource)
    {
        bool bRead = renderTargetResource->ReadPixels(bitmapOut, SensorUtilityReadPixelFlags);
        return bRead;
    }
    return false;
}

bool FSensorUtility::ReadPixels(FTextureRenderTargetResource* renderTargetRsc, TArray<FColor>& bitmapOut)
{
    if (renderTargetRsc)
    {
        bool bRead = renderTargetRsc->ReadPixels(bitmapOut, SensorUtilityReadPixelFlags);
        return bRead;
    }
    return false;
}

bool FSensorUtility::ReadPixelsLinear(FTextureRenderTargetResource* renderTargetRsc, TArray<FLinearColor>& bitmapOut)
{
    if (renderTargetRsc)
    {
        bool bRead = renderTargetRsc->ReadLinearColorPixels (bitmapOut, SensorUtilityReadPixelFlags);
        return bRead;
    }
    return false;
}

UTexture2D* FSensorUtility::MakeTextureBitmap(const int width, const int height, const TArray<FColor>& bitmap, UObject* InOuter)
{
    check((width * height) == bitmap.Num());

    // Decompress PNG image
    TArray64<uint8> RawData;

    // We are using the source art so grab the original width/height
    const int32 Width = width;
    const int32 Height = height;

    check(Width > 0 && Height > 0 && bitmap.Num() > 0);

    //UTexture2D* NewTexture2D = CreateTexture2D(Width, Height, EPixelFormat::PF_R32_FLOAT);
    UTexture2D* NewTexture2D = CreateTexture2D(Width, Height, InOuter, EPixelFormat::PF_R8G8B8A8);
    
    uint8* MipData = (uint8*)NewTexture2D->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
    for (int32 y = 0; y < Height; y++)
    {
        uint8* destPtr = &MipData[(Height - 1 - y) * Width * sizeof(FColor)];
        const FColor* SrcPtr = &((FColor*)(bitmap.GetData()))[(Height - 1 - y) * Width];
        for (int32 x = 0; x < Width; x++)
        {
            *destPtr++ = SrcPtr->R;
            *destPtr++ = SrcPtr->G;
            *destPtr++ = SrcPtr->B;
            *destPtr++ = SrcPtr->A;
            SrcPtr++;
        }
    }
    NewTexture2D->GetPlatformData()->Mips[0].BulkData.Unlock();

    // Set options
    NewTexture2D->SRGB = true;
#if WITH_EDITORONLY_DATA
    NewTexture2D->CompressionNone = true;
    NewTexture2D->MipGenSettings = TMGS_NoMipmaps;
#endif
    NewTexture2D->CompressionSettings = TC_Default;

    // Update the remote texture data
    NewTexture2D->UpdateResource();
    return NewTexture2D;
}

UTexture2D* FSensorUtility::MakeTextureBitmapLinearDepth(const int w, const int h, const TArray<FLinearColor>& bitmap, UObject* InOuter)
{
    check((w * h) == bitmap.Num());

    // Decompress PNG image
    TArray64<uint8> RawData;

    // We are using the source art so grab the original width/height
    const int32 Width = w;
    const int32 Height = h;

    check(Width > 0 && Height > 0 && bitmap.Num() > 0);

    //UTexture2D* NewTexture2D = CreateTexture2D(Width, Height, EPixelFormat::PF_R32_FLOAT);
    UTexture2D* NewTexture2D = CreateTexture2D(Width, Height, InOuter, EPixelFormat::PF_R8G8B8A8);
    
    uint8* MipData = (uint8*)NewTexture2D->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
    for (int32 y = 0; y < Height; y++)
    {
        uint8* destPtr = &MipData[(Height - 1 - y) * Width * sizeof(FColor)];
        const FLinearColor* SrcPtr = &((FLinearColor*)(bitmap.GetData()))[(Height - 1 - y) * Width];
        for (int32 x = 0; x < Width; x++)
        {
            *destPtr++ = FMath::Clamp(SrcPtr->R * 100, 0, 255);
            *destPtr++ = 0;
            *destPtr++ = 0;
            *destPtr++ = 1;
            SrcPtr++;
        }
    }
    NewTexture2D->GetPlatformData()->Mips[0].BulkData.Unlock();

    // Set options
    NewTexture2D->SRGB = true;
#if WITH_EDITORONLY_DATA
    NewTexture2D->CompressionNone = true;
    NewTexture2D->MipGenSettings = TMGS_NoMipmaps;
#endif
    NewTexture2D->CompressionSettings = TC_Default;

    // Update the remote texture data
    NewTexture2D->UpdateResource();
    return NewTexture2D;
}

UTexture2D* FSensorUtility::MakeTextureBitmapFillColor(const int width, const int height, const FColor& color, UObject* InOuter)
{
    check(width > 0 && height > 0);
    if (width <= 0 || height <= 0) return nullptr;

    //UTexture2D* NewTexture2D = CreateTexture2D(Width, Height, EPixelFormat::PF_R32_FLOAT);
    UTexture2D* newTexture2D = CreateTexture2D(width, height, InOuter, EPixelFormat::PF_R8G8B8A8);
    FillTextureBitmapColor(width, height, color, newTexture2D);
    return newTexture2D;
}

UTexture2D* FSensorUtility::MakeTextureBitmapFillArc(   const int width, const int height,
                                                            const FColor& colorBack, const FColor& colorArc,
                                                            const float arcAngle, const float arcRange, UObject* InOuter)
{
    UTexture2D* textureArc = MakeTextureBitmapFillColor(width, height, colorBack, InOuter);
    FillTextureBitmapArc(width, height, colorBack, colorArc, arcAngle, arcRange, textureArc);
    return textureArc;
}


EPixelFormat ReadRenderTargetHelper(
	TArray<FColor>& OutLDRValues,
	TArray<FLinearColor>& OutHDRValues,
	UObject* WorldContextObject,
	UTextureRenderTarget2D* TextureRenderTarget,
	int32 X,
	int32 Y,
	int32 Width,
	int32 Height)
{
	EPixelFormat OutFormat = PF_Unknown;

	if (!TextureRenderTarget)
	{
		return OutFormat;
	}

	FTextureRenderTarget2DResource* RTResource = (FTextureRenderTarget2DResource*)TextureRenderTarget->GameThread_GetRenderTargetResource();
	if (!RTResource)
	{
		return OutFormat;
	}

	X = FMath::Clamp(X, 0, TextureRenderTarget->SizeX - 1);
	Y = FMath::Clamp(Y, 0, TextureRenderTarget->SizeY - 1);
	Width = FMath::Clamp(Width, 1, TextureRenderTarget->SizeX);
	Height = FMath::Clamp(Height, 1, TextureRenderTarget->SizeY);
	Width = Width - FMath::Max(X + Width - TextureRenderTarget->SizeX, 0);
	Height = Height - FMath::Max(Y + Height - TextureRenderTarget->SizeY, 0);

	FIntRect SampleRect(X, Y, X + Width, Y + Height);
	FReadSurfaceDataFlags ReadSurfaceDataFlags;

	FRenderTarget* RenderTarget = TextureRenderTarget->GameThread_GetRenderTargetResource();
	OutFormat = TextureRenderTarget->GetFormat();

	const int32 NumPixelsToRead = Width * Height;

	switch (OutFormat)
	{
	case PF_B8G8R8A8:
		OutLDRValues.SetNumUninitialized(NumPixelsToRead);
		if (!RenderTarget->ReadPixelsPtr(OutLDRValues.GetData(), ReadSurfaceDataFlags, SampleRect))
		{
			OutFormat = PF_Unknown;
		}
		break;
	case PF_FloatRGBA:
		OutHDRValues.SetNumUninitialized(NumPixelsToRead);
		if (!RenderTarget->ReadLinearColorPixelsPtr(OutHDRValues.GetData(), ReadSurfaceDataFlags, SampleRect))
		{
			OutFormat = PF_Unknown;
		}
		break;
	default:
		OutFormat = PF_Unknown;
		break;
	}

	return OutFormat;
}

void FSensorUtility::FillTextureFromRT(UTextureRenderTarget2D* fromIn, UTexture2D* destOut)
{
    bool isValid = fromIn->SizeX == destOut->GetSizeX()
        && fromIn->SizeY == destOut->GetSizeY();
    ensure(isValid);
    if (isValid == false) return;
    int width = fromIn->SizeX, height = fromIn->SizeY;
    TArray<FColor> Samples;
    TArray<FLinearColor> LinearSamples;
    ReadRenderTargetHelper(Samples, LinearSamples, fromIn->GetWorld(), fromIn, 0, 0, width, height);
    
    uint8* destBuffer = (uint8*)destOut->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
    for (int32 y = 0; y < height; y++)
    {
        const FColor* srcColor = &((FColor*)(Samples.GetData()))[(height - 1 - y) * width];
        uint8* destPtr = &destBuffer[(height - 1 - y) * width * sizeof(FColor)];
        for (int32 x = 0; x < width; x++)
        {
            *destPtr++ = srcColor->R;
            *destPtr++ = srcColor->G;
            *destPtr++ = srcColor->B;
            *destPtr++ = srcColor->A;
            srcColor++;
        }
    }
    destOut->GetPlatformData()->Mips[0].BulkData.Unlock();
    
    // Set options
    destOut->SRGB = true;
#if WITH_EDITORONLY_DATA
    destOut->CompressionNone = true;
    destOut->MipGenSettings = TMGS_NoMipmaps;
#endif
    destOut->CompressionSettings = TC_Default;
    
    // Update the remote texture data
    destOut->UpdateResource();
}

void FSensorUtility::FillTextureBitmapColor(const int width, const int height, const FColor& color, UTexture2D* textureInOut)
{
    uint8* MipData = (uint8*)textureInOut->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
    for (int32 y = 0; y < height; y++)
    {
        uint8* destPtr = &MipData[(height - 1 - y) * width * sizeof(FColor)];
        for (int32 x = 0; x < width; x++)
        {
            *destPtr++ = color.R;
            *destPtr++ = color.G;
            *destPtr++ = color.B;
            *destPtr++ = color.A;
        }
    }
    textureInOut->GetPlatformData()->Mips[0].BulkData.Unlock();

    // Set options
    textureInOut->SRGB = true;
#if WITH_EDITORONLY_DATA
    textureInOut->CompressionNone = true;
    textureInOut->MipGenSettings = TMGS_NoMipmaps;
#endif
    textureInOut->CompressionSettings = TC_Default;

    // Update the remote texture data
    textureInOut->UpdateResource();
}

void FSensorUtility::FillTextureBitmapArc(  const int width,
                                                const int height,
                                                const FColor& colorBack,
                                                const FColor& colorArc,
                                                const float arcAngle,
                                                const float arcRange,
                                                UTexture2D* textureInOut)
{
    uint8* MipData = (uint8*)textureInOut->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
    
    const FVector2D center{ ((float)width / 2.0f), 0.0f };
    const FVector2D dirArc(0, 1);
    const FIntPoint cPnt{ (int)center.X, (int)center.Y };
    const float range = arcRange;
    for (int32 y = 0; y < height; y++)
    {
        for (int32 x = 0; x < width; x++)
        {
            uint8* destPtr = &MipData[(((height - 1 - y) * width) + x) * sizeof(FColor)];
            if (cPnt.X == x && cPnt.Y == y)
            {
                *destPtr++ = colorArc.R;
                *destPtr++ = colorArc.G;
                *destPtr++ = colorArc.B;
                *destPtr++ = colorArc.A;
                continue;
            }
            
            FVector2D pntCurrent{(float)x, (float)y};
            FVector2D dirCurrent = pntCurrent - center;
            float distance = dirCurrent.Size();
            ensure(distance!=0.0f);
            if (distance > (width / 2)) continue;
            dirCurrent.X = dirCurrent.X / distance;
            dirCurrent.Y = dirCurrent.Y / distance;
            float dot = dirCurrent.X * dirArc.X + dirCurrent.Y * dirArc.Y; 
            float angleCurrent = FMath::RadiansToDegrees(FMath::Acos(dot));
            if (angleCurrent > range) continue;
            *destPtr++ = colorArc.R;
            *destPtr++ = colorArc.G;
            *destPtr++ = colorArc.B;
            *destPtr++ = colorArc.A;
        }
    }
    textureInOut->GetPlatformData()->Mips[0].BulkData.Unlock();

    // Update the remote texture data
    textureInOut->UpdateResource();
}

UTexture2D* FSensorUtility::CreateTexture2D(int32 InSizeX, int32 InSizeY, UObject* InOuter, EPixelFormat InFormat, const FName InName)
{
    LLM_SCOPE(ELLMTag::Textures);

    UTexture2D* NewTexture = NULL;
    if (InSizeX > 0 && InSizeY > 0 &&
        (InSizeX % GPixelFormats[InFormat].BlockSizeX) == 0 &&
        (InSizeY % GPixelFormats[InFormat].BlockSizeY) == 0)
    {
        NewTexture = NewObject<UTexture2D>(InOuter, InName, RF_Public | RF_Standalone);
        NewTexture->SetPlatformData(new FTexturePlatformData());
        NewTexture->GetPlatformData()->SizeX = InSizeX;
        NewTexture->GetPlatformData()->SizeY = InSizeY;
        NewTexture->GetPlatformData()->PixelFormat = InFormat;

        // Allocate first mipmap.
        int32 NumBlocksX = InSizeX / GPixelFormats[InFormat].BlockSizeX;
        int32 NumBlocksY = InSizeY / GPixelFormats[InFormat].BlockSizeY;
        FTexture2DMipMap* Mip = new FTexture2DMipMap();
        NewTexture->GetPlatformData()->Mips.Add(Mip);
        Mip->SizeX = InSizeX;
        Mip->SizeY = InSizeY;
        Mip->BulkData.Lock(LOCK_READ_WRITE);
        Mip->BulkData.Realloc(NumBlocksX * NumBlocksY * GPixelFormats[InFormat].BlockBytes);
        Mip->BulkData.Unlock();
    }
    else
    {
        UE_LOG(LogTexture, Warning, TEXT("Invalid parameters specified for UTexture2D::CreateTransient()"));
    }
    return NewTexture;
}

UTexture2D* FSensorUtility::CreateTexture2DFromRenderTarget(UTextureRenderTarget2D* renderTarget, UObject* InOuter, EObjectFlags InObjectFlags)
{
    UTexture2D* newTexture = renderTarget->ConstructTexture2D(InOuter,
        TEXT("Clone_RenderTargetTexture2D"), InObjectFlags, CTF_Default, NULL);
    return newTexture;
}

UMaterialInterface* FSensorUtility::GetMaterialFromHitResult(FHitResult& hitResult)
{
    UPrimitiveComponent* comp = hitResult.GetComponent();
    ensure(comp);
    int32 sectionIndex = 0;
    int32 faceIndex = hitResult.FaceIndex;
    UMaterialInterface* materialInterface = comp->GetMaterialFromCollisionFaceIndex(faceIndex, sectionIndex);
    return materialInterface;
}

void FSensorUtility::DrawSphereMesh(ULineBatchComponent* lineBatcher, const FVector& center, const float radius, const FColor& color)
{
    ensure(lineBatcher);
    if (DefaultSphereVerts.Num() <= 0)
        DefaultSphereVerts.AddDefaulted(SphereDivCnt);
    if (DefaultSphereIdxs.Num() <= 0)
        DefaultSphereIdxs.AddDefaulted((SphereDivH - 1) * SphereDivV * 3 * 2 - 6);
    
    int idxVert = 0;
    int idxIdx = 0;
    for (int m = 0; m < SphereDivH; m++)
    {
        for (int n = 0; n < SphereDivV; n++)
        {
            int idx = idxVert++;
            float x = FMath::Sin(PI * m/SphereDivH) * FMath::Cos(2 * PI * n/SphereDivV);
            float y = FMath::Sin(PI * m/SphereDivH) * FMath::Sin(2 * PI * n/SphereDivV);
            float z = FMath::Cos(PI * m/SphereDivH);
            
            DefaultSphereVerts[idx] = (FVector(x, y, z) * radius) + center;
            if (m <= 0) continue;
            if (m == (SphereDivH - 1) && n == (SphereDivV - 1)) continue;
            DefaultSphereIdxs[idxIdx++] = idx - SphereDivV + 1;
            DefaultSphereIdxs[idxIdx++] = idx - SphereDivV + 0;
            DefaultSphereIdxs[idxIdx++] = idx + 0;

            DefaultSphereIdxs[idxIdx++] = idx - SphereDivV + 1;
            DefaultSphereIdxs[idxIdx++] = idx + 0;
            DefaultSphereIdxs[idxIdx++] = idx + 1;
        }
    }
    
    lineBatcher->DrawMesh(DefaultSphereVerts, DefaultSphereIdxs, color, DepthPriorityDraw, 1.0f);
    for (int i = 0; i < DefaultSphereVerts.Num(); ++i)
    {
        lineBatcher->DrawPoint(DefaultSphereVerts[i], FColor::MakeRandomColor(), 9.0f, FSensorUtility::DepthPriorityDraw, 10000.0f);
    }


    // TArray<FVector> verts;
    // TArray<int> Idxs;
    // verts.AddDefaulted(8);
    //
    // lineBatcher->DrawMesh(DefaultSphereVerts, DefaultSphereIdxs, color, DepthPriorityDraw, 1.0f);
}

void FSensorUtility::MakeDistortFisheyeSimple(int w, int h, float strength, const TArray<FColor>& bs, TArray<FColor>& bs_out)
{
    int num = w*h; 
    ensure(num == bs.Num());
    int width = w;
    int height = h;
    float centerX = static_cast<float>(width) / 2.0;
    float centerY = static_cast<float>(height) / 2.0;
    
    bs_out.AddDefaulted(num);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float deltaX = x - centerX;
            float deltaY = y - centerY;
            float radius = sqrt(deltaX * deltaX + deltaY * deltaY);

            if (radius > 0) {
                float theta = atan2(deltaY, deltaX);
                float newRadius = (radius) * strength * exp(-radius * 0.005);

                int newX = static_cast<int>(centerX + newRadius * cos(theta));
                int newY = static_cast<int>(centerY + newRadius * sin(theta));

                int idx = (y * width) + x;
                int idxNew = (newY * width) + newX;
                
                if (newX >= 0 && newX < width && newY >= 0 && newY < height) {
                    bs_out[idx] = bs[idxNew];
                }
            }
        }
    }
}
