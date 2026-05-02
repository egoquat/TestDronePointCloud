#pragma once

struct FSensorUtility
{
    struct FProfiler
    {
    private:
        int32 ID;
        static TMap<int32, FProfiler> Profilers;
        double TimeBegin;
        double TimeElapsed;
        
    public:

        static int32 CountingRay;
        static int32 CountingRaycast;
        
        void BeginInternal()
        {
            TimeBegin = FPlatformTime::Seconds();
        }

        double EndInternal()
        {
            return TimeElapsed += (FPlatformTime::Seconds() - TimeBegin);
        }

        FProfiler(const int32& id) { ID = id; TimeBegin = 0; TimeElapsed = 0; }
    
        static void Begin(const int32& id)
        {
            if (Profilers.Contains(id) == false)
            {
                Profilers.Add(id, FProfiler(id));            
            }
            Profilers[id].BeginInternal();
        }

        static double End(const int32& id)
        {
            if (Profilers.Contains(id) == false)
            {
                return 0;            
            }
            return Profilers[id].EndInternal();
        }

        static double GetTimeElapsed(const int32& id)
        {
            if (Profilers.Contains(id) == false)
            {
                return 0;            
            }
            return Profilers[id].TimeElapsed;
        }

        static void OutputProfilerDefault()
        {
            if (Profilers.Num() <= 0) return;
            FString output;
            int idx = 0;
            for (auto iter = Profilers.CreateIterator();iter; ++iter)
            {
                FProfiler& profiler = iter.Value();
                output += FString::Printf(TEXT("\t%d. Profile Measure id:%d, Elapsed:%f\n"), (idx++), profiler.ID, profiler.TimeElapsed);
            }
            GLog->Logf(TEXT("Profile Measure : \n%s"), *output);
        }

        static void Reset()
        {
            Profilers.Empty();
        }
    };

    static UTexture2D* LoadTexture(const FString& pathTexture);
    
    static void SaveTextureToDiskPixels(const FIntPoint& res, const TArray<FColor>& bitmap, const FString& FilePath);
    static void SavePixelsToDiskDepthTest(const FIntPoint& res, const TArray<FLinearColor>& bitmapDepth, const FString& FilePath);
    static void SaveTextureToDisk(UTextureRenderTarget2D* renderTargetTex, const FString& FilePath);
    static bool ReadPixels(UTextureRenderTarget2D* renderTargetTex, TArray<FColor>& bitmapOut);
    static bool ReadPixels(FTextureRenderTargetResource* renderTargetRsc, TArray<FColor>& bitmapOut);
    static bool ReadPixelsLinear(FTextureRenderTargetResource* renderTargetRsc, TArray<FLinearColor>& bitmapOut);

    static float GetLightColorIntensity(FLinearColor& color)
    {
        return GetLightColorIntensity(color.R, color.G, color.B);
    }
    
    static float GetLightColorIntensity(float r, float g, float b)
    {
        float lightintensity = ((0.3f * r) + (0.59f * g) + (0.11f * b)) / 3.0f;
        return lightintensity;
    }

    static UTexture2D* MakeTextureBitmap(const int w, const int h, const TArray<FColor>& bitmap, UObject* InOuter);
    static UTexture2D* MakeTextureBitmapLinearDepth(const int w, const int h, const TArray<FLinearColor>& bitmap, UObject* InOuter);
    static UTexture2D* MakeTextureBitmapFillColor(const int width, const int height, const FColor& color, UObject* InOuter);
    static UTexture2D* MakeTextureBitmapFillArc(const int width, const int height,
                                                const FColor& colorBack, const FColor& colorArc,
                                                const float arcAngle, const float arcRange, UObject* InOuter);
    static void FillTextureFromRT(UTextureRenderTarget2D* fromIn, UTexture2D* toOut);
    static void FillTextureBitmapColor(const int width, const int height, const FColor& color, UTexture2D* textureInOut);
    static void FillTextureBitmapArc(const int width, const int height,
                                     const FColor& colorBack, const FColor& colorArc,
                                     const float arcAngle, const float arcRange, UTexture2D* textureInOut);
    static UTexture2D* CreateTexture2D(int32 InSizeX, int32 InSizeY, UObject* InOuter, EPixelFormat InFormat = PF_B8G8R8A8, const FName InName = NAME_None);
    static UTexture2D* CreateTexture2DFromRenderTarget(UTextureRenderTarget2D* renderTarget, UObject* InOuter, EObjectFlags InObjectFlags = RF_Public | RF_Standalone);

    static UMaterialInterface* GetMaterialFromHitResult(FHitResult& hitResult);
    
    static void DrawSphereMesh(ULineBatchComponent* lineBatcher, const FVector& center, const float radius, const FColor& color);

    // 참고: https://chat.openai.com/c/041cf04e-0f0b-4b78-81ba-9b58027285ac
    static void MakeDistortFisheyeSimple(int w, int h, float strength, const TArray<FColor>& bs, TArray<FColor>& bs_out);

    inline const static uint8 DepthPriorityDraw = SDPG_Foreground;;
    inline const static int SphereDivH = 3;
    inline const static int SphereDivV = SphereDivH - 1;
    inline const static int SphereDivCnt = SphereDivH * SphereDivV;
    inline static TArray<FVector> DefaultSphereVerts;
    inline static TArray<int32> DefaultSphereIdxs;
};
