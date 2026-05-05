#pragma once

#include "TextureResource.h"
#include "Components/SceneCaptureComponent2D.h"
#include "SensorCaptureRay.h"

struct FSensorCapture
{
    int Index = INDEX_NONE;
    FIntPoint Res;
    float AngleYaw = 0;
    float AnglePitch = 0;
    float AngleYawRange = 0;
    float AnglePitchRange = 0;
    
    float AngleYawMin = 0;
    float AngleYawMax = 0;
    float AnglePitchMin = 0;
    float AnglePitchMax = 0;

    USceneCaptureComponent2D* CaptureComp = nullptr;
    UTextureRenderTarget2D* RenderTarget = nullptr;
    
    double TimeLastReadCapture = 0;
    FRotator SensorRotator;
    FTransform SensorTMLastCapture;
    FTransform SensorTMInverse;
    FMatrix ClipProjMatrix;
    
    TArray<FCaptureRay> CollectRays;
    TArray<FColor> Bitmap;
    TArray<FLinearColor> BitmapLinear;

    bool bHasDynamicCaptured = false;
    int32 CntCapturedFirstForce = 0;
    int32 IteratorIdxForArrangeRay = 0;

    static USceneCaptureComponent2D* MakeSceneCaptureRT(  const AActor* actorOwned,
                                                            const FRotator& relativeRotator,
                                                            float fovYaw,
                                                            int resX, int resY,
                                                            bool bCaptureEveryFrameRequest,
                                                            UTextureRenderTarget2D*& renderTargetOut,
                                                            FVector* relativePos = nullptr,
                                                            const ESceneCaptureSource* typeSceneCapture = nullptr);

    static USceneCaptureComponent2D* MakeSceneCaptureRTOption(
                                                            const AActor* actorOwned,
                                                            float fovYaw,
                                                            const FIntPoint& res,
                                                            bool bCaptureEveryFrameRequest,
                                                            ESceneCaptureSource typeFinalRT,
                                                            const FRotator& relativeRotator,
                                                            FVector* relativePos = nullptr);

    static USceneCaptureComponent2D* MakeSceneCapture(  const AActor* actorOwned,
                                                        const FRotator& relativeRotator,
                                                        float fovYaw,
                                                        int resX, int resY,
                                                        bool bCaptureEveryFrameRequest,
                                                        UTextureRenderTarget2D* renderTarget,
                                                        FVector* relativePos = nullptr,
                                                        const ESceneCaptureSource* typeSceneCapture = nullptr);

    static UTextureRenderTarget2D* MakeRT(const AActor* actorOwned, int resX, int resY);
    
    static float NormalizeAngle(float angle) {
        return FMath::Fmod((angle >= 0 ? angle : (360.0f - FMath::Fmod(-angle, 360))), 360);
    }

    bool IsAngleYawRange(const float angleYawRayIn);
    bool IsAngleYawRange(const float angleMin, const float angleMax);
    
    void OnTickInitializeCapture()
    {
        FRotator sensorRotator = CaptureComp->GetOwner()->GetActorRotation();
        sensorRotator.Yaw = sensorRotator.Yaw + AngleYaw;
        sensorRotator.Pitch = sensorRotator.Pitch + AnglePitch;
        SensorRotator = sensorRotator;
        MakeFOVProjectionClippingMatrix(Res,
            AngleYawRange * PI / 360.0f,
            10, 1 << 14,
            ClipProjMatrix);
    }

    static bool IsSimilar(const FTransform& tm1, const FTransform& tm2)
    {
        const static float ToleranceLocation = 1.0f; // Centimeter
        const static float ToleranceAngle = 0.2f; // Degree
        bool isSameAlmost = false;
        isSameAlmost = tm1.GetLocation().Equals(tm2.GetLocation(), ToleranceLocation)
            && tm1.Rotator().Equals(tm2.Rotator(), ToleranceAngle);
        return isSameAlmost;
    }
    
    static void GetUnpackColorRGBA(uint32 packedColor, FColor& colorOut)
    {
        colorOut.R = (packedColor >> 24) & 0xff;
        colorOut.G = (packedColor >> 16) & 0xff;
        colorOut.B = (packedColor >> 8) & 0xff;
        colorOut.A = packedColor & 0xff;
    }

    static void GetUnpackLinearColorRGBA(int32 packedColor, float& r, float& g, float& b, float& a)
    {
        r = ((packedColor >> 24) & (0xff)) / (float)0xff;
        g = ((packedColor >> 16) & (0xff)) / (float)0xff;
        b = ((packedColor >> 8) & (0xff)) / (float)0xff;
        a = ((packedColor & (0xff)) / (float)0xff);
    }

    void CaptureRequestWithLinear() { CaptureRequest(false, false, true); }
    void CaptureRequest(bool bReadAlways = false, bool bCaptureForce = false, bool bReadLinearDepthToBitmap = false);
    void ArrangeRay(FCaptureRay& cast, int& indexPack_out);
    FCaptureRay* GetArrangedRay(const FHitResult& rayHit, int& indexPack_out);
    
    float GetAngYaw(float angYawActor) { return AngleYaw + angYawActor; }
    float GetAngPitch(float angPitchActor) { return AnglePitch + angPitchActor; }

    bool GetColorFromCapture(    const FRotator& rotatorRay,
                                        const FVector& locHit,
                                        const TArray<FColor>& bitmap,
                                        FColor& color_out,
                                        FIntPoint& uv_out);
    
    static void MakeFOVProjectionClippingMatrix(const FIntPoint& res, float FOVYaw, float InNearClippingPlane, float InFarClippingPlane, FMatrix& ProjectionMatrix);

    explicit FSensorCapture(  USceneCaptureComponent2D* captureComp);
    
    explicit FSensorCapture(  const int index,
                            const FIntPoint& res,
                            const float fovYaw,
                            const float AngleYawDefault,
                            const float AnglePitchDefault,
                            USceneCaptureComponent2D* captureComp,
                            UTextureRenderTarget2D* renderTarget);
    explicit FSensorCapture();
};
