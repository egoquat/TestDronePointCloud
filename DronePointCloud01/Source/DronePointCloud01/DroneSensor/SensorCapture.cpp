#include "SensorCapture.h"
#include "SensorUtility.h"
#include "CustomShaderUtility.h"
#include "SensorUIVisualizer.h"
#include "Engine/TextureRenderTarget2D.h"

static int GSensorCapturePerspectiveByPrecision = 1;
FAutoConsoleVariableRef CVarCapturePerspectiveByPrecision(
TEXT("DPC.Sensor.CapturePerspectiveByPrecision"),
GSensorCapturePerspectiveByPrecision,
TEXT("default ON = 1\n")
TEXT("OFF = 0\n"),
ECVF_Cheat
);

using FProfiler = FSensorUtility::FProfiler;

static int32 StaticCountMakeCapture = 0;
USceneCaptureComponent2D* FSensorCapture::MakeSceneCaptureRT(   const AActor* actorOwned,
                                                                const FRotator& relativeRotator,
                                                                float fovYaw,
                                                                int resX, int resY,
                                                                bool bCaptureEveryFrame,
                                                                UTextureRenderTarget2D*& renderTargetOut,
                                                                FVector* relativePos,
                                                                const ESceneCaptureSource* typeSceneCapture)
{
    renderTargetOut = MakeRT(actorOwned, resX, resY);
    USceneCaptureComponent2D* capture = NewObject<USceneCaptureComponent2D>(actorOwned->GetRootComponent(), USceneCaptureComponent2D::StaticClass());
    FAttachmentTransformRules attachmentRules(EAttachmentRule::KeepRelative, false);
    capture->AttachToComponent(actorOwned->GetRootComponent(), attachmentRules);
    capture->SetRelativeRotation(relativeRotator);
    if (relativePos != nullptr)
        capture->SetRelativeLocation(*relativePos);

    capture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
    capture->bCaptureOnMovement = true;
    capture->bCaptureEveryFrame = bCaptureEveryFrame;
    capture->bAlwaysPersistRenderingState = true;
    capture->bEnableClipPlane = true;
    
    capture->TextureTarget = renderTargetOut;
    capture->CaptureSource = typeSceneCapture == nullptr ? ESceneCaptureSource::SCS_SceneColorSceneDepth : *typeSceneCapture;
    capture->FOVAngle = fovYaw;
    
    //capture->UpdateContent();
    capture->RegisterComponent();
    //capture->Activate(true);
    ++StaticCountMakeCapture;
    GLog->Logf(TEXT(">>>>>>>>> Make CaptureComponent %d"), StaticCountMakeCapture);
    return capture;
}

USceneCaptureComponent2D* FSensorCapture::MakeSceneCaptureRTOption( const AActor* actorOwned,
                                                                    float fovYaw,
                                                                    const FIntPoint& res,
                                                                    bool bCaptureEveryFrame,
                                                                    ESceneCaptureSource typeFinalRT,
                                                                    const FRotator& relativeRotator,
                                                                    FVector* relativePos)
{
    UTextureRenderTarget2D* renderTarget = MakeRT(actorOwned, res.X, res.Y);
    USceneCaptureComponent2D* capture = NewObject<USceneCaptureComponent2D>(actorOwned->GetRootComponent(), USceneCaptureComponent2D::StaticClass());
    FAttachmentTransformRules attachmentRules(EAttachmentRule::KeepRelative, false);
    capture->AttachToComponent(actorOwned->GetRootComponent(), attachmentRules);
    capture->SetRelativeRotation(relativeRotator);
    if (relativePos != nullptr)
        capture->SetRelativeLocation(*relativePos);
    
    capture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
    capture->bCaptureOnMovement = true;
    capture->bCaptureEveryFrame = bCaptureEveryFrame;
    capture->bAlwaysPersistRenderingState = true;
    capture->bEnableClipPlane = true;
    
    capture->TextureTarget = renderTarget;
    capture->CaptureSource = typeFinalRT;
    capture->FOVAngle = fovYaw;
    
    capture->RegisterComponent();
    ++StaticCountMakeCapture;
    return capture;
}

USceneCaptureComponent2D* FSensorCapture::MakeSceneCapture(   const AActor* actorOwned,
                                                            const FRotator& relativeRotator,
                                                            float fovYaw,
                                                            int resX, int resY,
                                                            bool bCaptureEveryFrame,
                                                            UTextureRenderTarget2D* renderTarget,
                                                            FVector* relativePos,
                                                            const ESceneCaptureSource* typeSceneCapture)
{    
    USceneCaptureComponent2D* capture = NewObject<USceneCaptureComponent2D>(actorOwned->GetRootComponent(), USceneCaptureComponent2D::StaticClass());
    FAttachmentTransformRules attachmentRules(EAttachmentRule::KeepRelative, false);
    capture->AttachToComponent(actorOwned->GetRootComponent(), attachmentRules);
    capture->SetRelativeRotation(relativeRotator);
    if (relativePos != nullptr)
        capture->SetRelativeLocation(*relativePos);

    capture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
    capture->bCaptureOnMovement = true;
    capture->bCaptureEveryFrame = bCaptureEveryFrame;
    capture->bAlwaysPersistRenderingState = true;
    capture->bEnableClipPlane = true;
    
    capture->TextureTarget = renderTarget;
    capture->CaptureSource = typeSceneCapture == nullptr ? ESceneCaptureSource::SCS_SceneDepth : *typeSceneCapture;
    capture->FOVAngle = fovYaw;
    
    //capture->UpdateContent();
    capture->RegisterComponent();
    //capture->Activate(true);
    ++StaticCountMakeCapture;
    GLog->Logf(TEXT(">>>>>>>>> Make CaptureComponent %d"), StaticCountMakeCapture);
    return capture;
}

UTextureRenderTarget2D* FSensorCapture::MakeRT(const AActor* actorOwned, int resX, int resY)
{
    UTextureRenderTarget2D* renderTarget = NewObject<UTextureRenderTarget2D>(actorOwned->GetRootComponent());
    //renderTargetOut->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA16f;
    renderTarget->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8_SRGB;
    renderTarget->CompressionSettings = TextureCompressionSettings::TC_Default;
    renderTarget->SRGB = false;
    renderTarget->bAutoGenerateMips = false;
    renderTarget->bGPUSharedFlag = true;
    renderTarget->AddressX = TextureAddress::TA_Clamp;
    renderTarget->AddressY = TextureAddress::TA_Clamp;
    renderTarget->InitAutoFormat(resX, resY);
    renderTarget->UpdateResourceImmediate(true);
    return renderTarget;
}

bool FSensorCapture::IsAngleYawRange(const float angleYawRayIn)
{
    static const float EPSILONLow = 0.01f;
    float angleYawMin = AngleYawMin + SensorRotator.Yaw - EPSILONLow;
    float angleYawMax = AngleYawMax + SensorRotator.Yaw + EPSILONLow;
    float angleYawRay = angleYawRayIn;

    float angleYawMinNorm = NormalizeAngle(angleYawMin);
    float angleYawMaxNorm = NormalizeAngle(angleYawMax);
    float angleYawRayNorm = NormalizeAngle(angleYawRay);

    bool bCrossed = angleYawMaxNorm < angleYawMinNorm;
    if (bCrossed == true)
    {
        return angleYawMinNorm <= angleYawRayNorm && (360.0f + EPSILONLow) >= angleYawRayNorm
            || 0.0f <= angleYawRayNorm && angleYawMaxNorm >= angleYawRayNorm; 
    }
    
    return angleYawMinNorm <= angleYawRayNorm && angleYawMaxNorm >= angleYawRayNorm;
}

bool FSensorCapture::IsAngleYawRange(const float angleMin, const float angleMax)
{
    return IsAngleYawRange(angleMin) || IsAngleYawRange(angleMax);
}

void FSensorCapture::ArrangeRay(FCaptureRay& cast, int& indexPack_out)
{
    bHasDynamicCaptured |= (cast.SemanticTag == 4u || cast.SemanticTag == 10u);
    IteratorIdxForArrangeRay = FMath::Min(IteratorIdxForArrangeRay, CollectRays.Num() - 1);
    int32 idxArrange = IteratorIdxForArrangeRay++;
    int32 indexPack = cast.Hit.Item = (Index | (idxArrange << 8));
    indexPack_out = indexPack;
    CollectRays[idxArrange] = cast;
}

FCaptureRay* FSensorCapture::GetArrangedRay(const FHitResult& rayHit, int& indexPack_out)
{
    IteratorIdxForArrangeRay = FMath::Min(IteratorIdxForArrangeRay, CollectRays.Num() - 1);
    int32 idxArranged = IteratorIdxForArrangeRay++;
    int32 indexPack = (Index | (idxArranged << 8));
    indexPack_out = indexPack;
    FCaptureRay* rayArranged = &CollectRays[idxArranged];
    return rayArranged;
}

bool FSensorCapture::GetColorFromCapture(const FRotator& rotatorRay, const FVector& locHit, const TArray<FColor>& bitmap, FColor& color_out, FIntPoint& uv_out)
{
    ensure(bitmap.Num()>=1);
    FVector locHit2 = SensorTMInverse.TransformPosition(locHit);
    FVector locHitProj(locHit2.Y, locHit2.Z, locHit2.X);
    locHitProj = ClipProjMatrix.TransformPosition(locHitProj);
    locHitProj = locHitProj.Projection();

    float ratioX = (locHitProj.X + 1.0f) * 0.5f;
    float ratioY = 1.0f - (locHitProj.Y + 1.0f) * 0.5f;

    //if (ratioX < 0 || ratioX > 1.0f) return false;
    //if (ratioY < 0 || ratioY > 1.0f) return false;
    
    ratioX = FMath::Clamp(ratioX, 0.0f, 1.0f);
    ratioY = FMath::Clamp(ratioY, 0.0f, 1.0f);
    
    int x = FMath::Floor(ratioX * (float)(Res.X - 1));
    int y = FMath::Floor(ratioY * (Res.Y - 1));

    color_out = bitmap[RenderTarget->SizeX * y + x];
    uv_out.X = x; uv_out.Y = y;

    return true;
}

void FSensorCapture::MakeFOVProjectionClippingMatrix(const FIntPoint& res, float FOVYaw, float InNearClippingPlane, float InFarClippingPlane, FMatrix& ProjectionMatrix)
{
    if (GSensorCapturePerspectiveByPrecision == 1)
    {
        ProjectionMatrix = FReverseZClipPerspectiveMatrix(
           FOVYaw,
           res.X,
           res.Y,
           InNearClippingPlane,
           InFarClippingPlane
           );   
    }
    else
    {
        ProjectionMatrix = FClipPerspectiveMatrix(
            FOVYaw,
            res.X,
            res.Y,
            InNearClippingPlane,
            InFarClippingPlane
        );
    }
}

FSensorCapture::FSensorCapture(USceneCaptureComponent2D* captureComp)
        :   Index(-1),
            Res(FIntPoint(captureComp->TextureTarget->GetSurfaceWidth(), captureComp->TextureTarget->GetSurfaceHeight())),
            AngleYawRange(captureComp->FOVAngle),
            AnglePitchRange(AngleYawRange * ((float)Res.Y / (float)Res.X)),
            SensorTMLastCapture(FTransform::Identity)
{
    UTextureRenderTarget2D* renderTarget = captureComp->TextureTarget;
    RenderTargetResource = renderTarget != nullptr ? renderTarget->GameThread_GetRenderTargetResource() : nullptr;
    AngleYawMin = -(captureComp->FOVAngle / 2);
    AngleYawMax = AngleYawMin + captureComp->FOVAngle;
    AnglePitchMin = AnglePitch + (-(AnglePitchRange / 2));
    AnglePitchMax = AnglePitchMin + AnglePitchRange;
    CaptureComp = captureComp;
    if (renderTarget == nullptr)
    {
        if (captureComp != nullptr)
        {
            RenderTarget = captureComp->TextureTarget;
        }
    }
    else
    {
        RenderTarget = renderTarget;
    }
}

FSensorCapture::FSensorCapture( const int index,
                            const FIntPoint& res,
                            const float fovYaw,
                            const float AngleYawDefault,
                            const float AnglePitchDefault,
                            USceneCaptureComponent2D* captureComp,
                            UTextureRenderTarget2D* renderTarget)
        :   Index(index), Res(res),
            AngleYaw(AngleYawDefault),
            AnglePitch(AnglePitchDefault),
            AngleYawRange(fovYaw),
            AnglePitchRange(fovYaw * ((float)res.Y / (float)res.X)),
            SensorTMLastCapture(FTransform::Identity)
{
    RenderTargetResource = renderTarget != nullptr ? renderTarget->GameThread_GetRenderTargetResource() : nullptr;
    AngleYawMin = -(fovYaw / 2);
    AngleYawMax = AngleYawMin + fovYaw;
    AnglePitchMin = AnglePitch + (-(AnglePitchRange / 2));
    AnglePitchMax = AnglePitchMin + AnglePitchRange;
    CaptureComp = captureComp;
    if (renderTarget == nullptr)
    {
        if (captureComp != nullptr)
        {
            RenderTarget = captureComp->TextureTarget;
        }
    }
    else
    {
        RenderTarget = renderTarget;
    }
}

FSensorCapture::FSensorCapture()
{
}
