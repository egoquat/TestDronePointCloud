#include "SensorUIVisualizer.h"

#include "Blueprint/WidgetBlueprintLibrary.h"

void FSensorVisualizer::DrawGraph(  UCanvas*                  Canvas,
                                        float                     GraphX,
                                        float                     GraphY,
                                        float                     GraphWidth,
                                        float                     GraphHeight,
                                        FDrawGraphInstance&       graphInstance)
{
	DrawGraph(  Canvas,
                GraphX,
                GraphY,
                GraphWidth,
                GraphHeight,
                graphInstance.GraphPoints,
                graphInstance.MinValue,
                graphInstance.MaxValue,
                graphInstance.NameGraph,
                graphInstance.LineColor);
}

void FSensorVisualizer::DrawGraph(  UCanvas*                  Canvas,
                                        float                     GraphX,
                                        float                     GraphY,
                                        float                     GraphWidth,
                                        float                     GraphHeight,
                                        const std::vector<float>& points,
                                        const float               min,
                                        const float               max,
                                        const FString&            nameChart,
                                        const FLinearColor&       lineColor)
{
	ensure(points.size() <= FDrawGraphInstance::LimitChartPoint);

    float half = fmax(abs(min), abs(max));
	const float minY = -half;
	const float maxY = half;

	float graphY[FDrawGraphInstance::LimitChartPoint];
	FMemory::Memset(&graphY, 0, sizeof(float) * FDrawGraphInstance::LimitChartPoint);

	for (int i = 0; i < points.size(); ++i)
	{
		float y = points[i];
		graphY[i] = y;
	}

	FString Label = nameChart + FString::Printf(TEXT("[%.2f,%.2f]"), minY, maxY);
	Canvas->SetDrawColor(FColor::Black);
	UFont* Font = GEngine->GetMediumFont();
	Canvas->DrawText(Font, Label, GraphX, GraphY);

	float XL, YL;
	Canvas->TextSize(Font, Label, XL, YL);

	float LineGraphHeight = GraphHeight - YL - 4.0f;
	float LineGraphY = GraphY + YL + 4.0f;

	FCanvasTileItem TileItem(FVector2D(GraphX, LineGraphY), GWhiteTexture, FVector2D(GraphWidth, LineGraphHeight), FLinearColor(0.0f, 0.125f, 0.0f, 0.25f));
	TileItem.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(TileItem);
	Canvas->SetDrawColor(FColor(0, 32, 0, 128));
	float rangeY = maxY - minY;
	for (uint32 i = 1; i < FDrawGraphInstance::LimitChartPoint; ++i)
	{
		float x1 = (float)(i - 1) / (float)(FDrawGraphInstance::LimitChartPoint - 1);
		float y1 = ((graphY[i - 1] - minY) / rangeY);
		float x2 = (float)i / (float)(FDrawGraphInstance::LimitChartPoint - 1);
		float y2 = ((graphY[i] - minY) / rangeY);

		x1 = FMath::Clamp(x1, 0.0f, 1.0f);
		x2 = FMath::Clamp(x2, 0.0f, 1.0f);
		y1 = FMath::Clamp(y1, 0.0f, 1.0f) - 0.5f;
		y2 = FMath::Clamp(y2, 0.0f, 1.0f) - 0.5f;

		FCanvasLineItem LineItem(FVector2D(GraphX + x1 * GraphWidth,
			                        LineGraphY + (LineGraphHeight * 0.5f) + (y1 * LineGraphHeight)),
			                    FVector2D(GraphX + x2 * GraphWidth,
				                    LineGraphY + (LineGraphHeight * 0.5f) + y2 * LineGraphHeight));
		LineItem.SetColor(lineColor);
		LineItem.Draw(Canvas->Canvas);
	}

	//OutX = FMath::Max(XL,GraphWidth);
}

void FSensorVisualizer::DrawGraphOnWidget(  FPaintContext& context,
                                                float GraphX,
                                                float GraphY,
                                                float GraphWidth,
                                                float GraphHeight,
                                                FDrawGraphInstance& graphInstance )
{
    DrawGraphOnWidget(  context,
                        GraphX,
                        GraphY,
                        GraphWidth,
                        GraphHeight,
                        graphInstance.GraphPoints,
                        graphInstance.MinValue,
                        graphInstance.MaxValue,
                        graphInstance.NameGraph,
                        graphInstance.LineColor);
}

void FSensorVisualizer::DrawGraphOnWidget(FPaintContext& context,
                            float GraphX,
                            float GraphY,
                            float GraphWidth,
                            float GraphHeight,
                            const std::vector<float>& points,
                            const float min,
                            const float max,
                            const FString& nameChart,
                            const FLinearColor& lineColor)
{
    ensure(points.size() <= FDrawGraphInstance::LimitChartPoint);

    float half = fmax(abs(min), abs(max));
    const float minY = -half;
    const float maxY = half;

    float graphY[FDrawGraphInstance::LimitChartPoint];
    FMemory::Memset(&graphY, 0, sizeof(float) * FDrawGraphInstance::LimitChartPoint);

    for (int i = 0; i < points.size(); ++i)
    {
        float y = points[i];
        graphY[i] = y;
    }

    FString Label = nameChart + FString::Printf(TEXT("[%.2f,%.2f]"), minY, maxY);
    
    UFont* font = GEngine->GetMediumFont();
    //Canvas->DrawText(Font, Label, GraphX, GraphY);
    UWidgetBlueprintLibrary::DrawTextFormatted(context, FText::FromString(Label), FVector2D(GraphX, GraphY), font);
    
    float LineGraphHeight = GraphHeight - 14.0f;
    float LineGraphY = GraphY + 14.0f;

    TArray<FVector2D> lines;
    lines.SetNum(FDrawGraphInstance::LimitChartPoint);
    float rangeY = maxY - minY;
    for (uint32 i = 0; i < FDrawGraphInstance::LimitChartPoint; ++i)
    {
        float xChartPnt = (float)i / (float)(FDrawGraphInstance::LimitChartPoint - 1);
        float yChartPnt = ((graphY[i] - minY) / rangeY);

        xChartPnt = FMath::Clamp(xChartPnt, 0.0f, 1.0f);
        yChartPnt = FMath::Clamp(yChartPnt, 0.0f, 1.0f) - 0.5f;

        xChartPnt = GraphX + xChartPnt * GraphWidth;
        yChartPnt = LineGraphY + (LineGraphHeight * 0.5f) + yChartPnt * LineGraphHeight;
        lines[i].X = xChartPnt; lines[i].Y = yChartPnt; 
    }

    UWidgetBlueprintLibrary::DrawLines(context, lines, lineColor);
}

FColor FSensorVisualizer::GetColorLerpRGB(float ratio)
{
    const static float clipped = 0.3f;
    const static float multiplier = 1.0f / (1.0f - clipped);
    float r,g,b; r=g=b=0;
    r = FMath::Clamp((ratio - clipped) * multiplier, 0.0f, 1.0f);
    g = (0.5f - std::abs(ratio - 0.5f)) * 2.0f;
    b = FMath::Clamp((1.0f - ratio - clipped) * multiplier, 0.0f, 1.0f);
    return FColor(r*255.0f,g*255.0f,b*255.0f,255);
}

FColor FSensorVisualizer::GetColorLerp(const float ratio, const TArray<FColorRatio>& colorTable)
{
    ensure(colorTable.Num() >= 2);
    int32 i01 = INDEX_NONE;
    float r01 = 0;
    for (int i = 1; i < colorTable.Num(); ++i)
    {
        const FColorRatio& cr = colorTable[i];
        float r02 = cr.Ratio;
        if (ratio >= r01 && ratio <= r02)
        {
            i01 = i - 1; break;
        }
        r01 = r02;
    }
    
    if(i01 == INDEX_NONE)
    {
        return colorTable[0].Co;        
    }
    const FColorRatio& c01 = colorTable[i01], c02 = colorTable[i01+1];
    FLinearColor lco = FLinearColor::LerpUsingHSV(c01.Co, c02.Co, (ratio - c01.Ratio) / (c02.Ratio - c01.Ratio)); 
    return lco.ToFColor(true);
}
