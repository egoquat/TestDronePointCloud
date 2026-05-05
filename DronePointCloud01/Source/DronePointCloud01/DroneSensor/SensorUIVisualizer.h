#pragma once
#include <vector>

#include "SensorCommon.h"
#include "Engine/Canvas.h"
#include "SensorDescription.h"
#include "Blueprint/UserWidget.h"

struct FDrawGraphInstance
{
    static const int32 LimitChartPoint = 100;
    static const int32 LimitCacheChartPoint = LimitChartPoint * 3;
    
    int Index = 0;
    float MinValue = -1.0f;
    float MaxValue = 1.0f;
    int IndexPoint = -1;
    int IndexPointCache = -1;
    int IndexCol = 0;
    FString NameGraph;
    FLinearColor LineColor;
    std::vector<float> GraphPoints;
    std::vector<float> CacheGraphPoints;

    void UpdatePoint(float pointValue)
    {
        IndexPoint = FMath::Clamp(IndexPoint + 1, 0, LimitChartPoint - 1);
        IndexPointCache = FMath::Clamp(IndexPointCache + 1, 0, LimitCacheChartPoint - 1);

        if (IndexPoint >= 1)
        {
            std::copy(GraphPoints.begin() + 1, GraphPoints.end(), GraphPoints.begin());
            std::copy(CacheGraphPoints.begin() + 1, CacheGraphPoints.end(), CacheGraphPoints.begin());
        }
        
        GraphPoints[IndexPoint] = pointValue;
        CacheGraphPoints[IndexPointCache] = pointValue;
    }

    void UpdatePointWithMinMax(float point)
    {
        UpdatePoint(point);

        float minValueCached = 0.0f;
        float maxValueCached = 1.0f;

        for (int i = 0; i <= IndexPointCache; ++i)
        {
            float value = CacheGraphPoints[i];
            minValueCached = FMath::Min(minValueCached, value);
            maxValueCached = FMath::Max(maxValueCached, value);
        }
        
        MinValue = minValueCached;
        MaxValue = maxValueCached;
    }
    
    FDrawGraphInstance(int index, int indexCol, const FColor& lineColor, const FString& nameGraph)
    {
        Index = index;
        IndexCol = indexCol;
        LineColor = lineColor;
        GraphPoints.resize(LimitChartPoint);
        CacheGraphPoints.resize(LimitCacheChartPoint);
        NameGraph = nameGraph;
    }
};

struct FAABB
{
    FVector Min;
    FVector Max;
    FAABB(FVector min, FVector max) { Min = min; Max = max; }
};

struct FDrawMinimap
{
    inline static const uint32 MinimapResXDefault = 200;
    inline static const FVector2D MinimapScreenPoint = FVector2D(340.0f, 20.0f);
    inline static const TArray<TTuple<FString, FString, FAABB>> PathGnss2DMinimapTests {
        TTuple<FString, FString, FAABB>(TEXT("Town03_Opt"), TEXT("/Game/Test/TestGnssMap01/Town03_opt_topview_01"),
            FAABB(FVector(-19140,-26250,0), FVector(28280,26780,0))),
        TTuple<FString, FString, FAABB>(TEXT("Pangyo"), TEXT("/Game/Test/TestGnssMap01/Pangyo_topview_01"),
            FAABB(FVector(-33110,-17070,0), FVector(73180,115120,0))),
    }; 
    
    struct FMinimapUnit
    {
        const FDrawMinimap* Minimap = nullptr;
        const AActor* Vehicle;
        void GetPointOnMinimap();
        FMinimapUnit(const FDrawMinimap* minimap, const AActor* vehicle)
        {
            Minimap = minimap;
            Vehicle = vehicle;
        }
    };
    
    UTexture* TextureMinimapDraw = nullptr;
    FVector2D MinimapSize;
    FVector WorldMin;
    FVector WorldMax;
    FVector2D WorldSize;
    FVector2D ScreenPoint;
    uint32 MinimapResX = 0;
    float MinimapScale = 1.0f;
    std::vector<FMinimapUnit> Units;

    bool IsReady() { return TextureMinimapDraw != nullptr; }

    FVector2D To2DDirectly(const FVector& vector)
    {        
        return FVector2D(vector);
    }

    FVector2D To2DPointNormalize(const FVector& worldLoc)
    {
        FVector relativeLoc = (worldLoc - WorldMin); 
        FVector2D point = To2DDirectly(relativeLoc);
        float x = FMath::Clamp(point.X / WorldSize.X, 0.0f, 1.0f);
        float y = FMath::Clamp(point.Y / WorldSize.Y, 0.0f, 1.0f);
        point.X = x; point.Y = y;
        return point;
    }

    // unitNormalizedcPoints : 0.0 ~ 1.0
    void GetCurrentNormalizedPoints(std::vector<FVector2D>& unitNormalizePoints)
    {
        unitNormalizePoints.clear();
        for (int i = 0; i < Units.size(); ++i)
        {
            FMinimapUnit& unit = Units[i];
            FVector worldLoc = unit.Vehicle->GetActorLocation();
            FVector2D pointNormalize = To2DPointNormalize(worldLoc);
            unitNormalizePoints.push_back(pointNormalize);
        }
    }

    void GetCurrentMinimapPoints(std::vector<FVector2D>& unitMinimapPoints)
    {
        unitMinimapPoints.clear();
        for (int i = 0; i < Units.size(); ++i)
        {
            FMinimapUnit& unit = Units[i];
            FVector worldLoc = unit.Vehicle->GetActorLocation();
            FVector2D pointMinimap = To2DPointNormalize(worldLoc);
            pointMinimap.X = ScreenPoint.X + (pointMinimap.X * MinimapSize.X);
            pointMinimap.Y = ScreenPoint.Y + (pointMinimap.Y * MinimapSize.Y);
            unitMinimapPoints.push_back(pointMinimap);
        }
    }

    void UpdateScreenPoint(int posX, int posY)
    {
        ScreenPoint = FVector2D(posX, posY);
    }
    
    void AddUnit(const AActor* vehicle)
    {
        Units.push_back(FMinimapUnit(this, vehicle));
    }

    void InitPosTexture(const uint32 resX,
                const FVector2D& screenPoint)
    {
        if (TextureMinimapDraw != nullptr)
        {
            MinimapSize = FVector2D(TextureMinimapDraw->GetSurfaceWidth(), TextureMinimapDraw->GetSurfaceHeight());   
        }
        float scaleInv = (float)resX / MinimapSize.X; 
        MinimapSize.X = MinimapSize.X * scaleInv;
        MinimapSize.Y = MinimapSize.Y * scaleInv;
        MinimapResX = resX;
        MinimapScale = scaleInv;
        ScreenPoint = screenPoint;
    }
    
    void Init(  UTexture* textureMinimapDraw,
                const FVector& worldMin,
                const FVector& worldMax,
                const uint32 resX,
                const FVector2D& screenPoint)
    {
        TextureMinimapDraw = textureMinimapDraw;
        WorldMin = worldMin; WorldMax = worldMax;
        WorldSize = To2DDirectly(WorldMax - WorldMin);
        
        InitPosTexture(resX, screenPoint);
    }
};

struct FSensorVisualizer
{
    static void DrawGraph(  UCanvas* Canvas,
                            float GraphX,
                            float GraphY,
                            float GraphWidth,
                            float GraphHeight,
                            FDrawGraphInstance& graphInstance);
                            
    static void DrawGraph(  UCanvas* Canvas,
                        float GraphX,
                        float GraphY,
                        float GraphWidth,
                        float GraphHeight,
                        const std::vector<float>& points,
                        const float min,
                        const float max,
                        const FString& nameChart,
                        const FLinearColor& lineColor );

    static void DrawGraphOnWidget( FPaintContext& context,
                            float GraphX,
                            float GraphY,
                            float GraphWidth,
                            float GraphHeight,
                            FDrawGraphInstance& graphInstance);
                            
    static void DrawGraphOnWidget( FPaintContext& context,
                            float GraphX,
                            float GraphY,
                            float GraphWidth,
                            float GraphHeight,
                            const std::vector<float>& points,
                            const float min,
                            const float max,
                            const FString& nameChart,
                            const FLinearColor& lineColor );

    static FColor GetColorLerpRGB(float ratio);
    
    static FColor GetColorLerp(const float ratio, const TArray<FColorRatio>& colorTable);
};
