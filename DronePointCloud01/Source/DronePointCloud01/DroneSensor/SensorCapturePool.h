#pragma once
#include "Components/SceneCaptureComponent2D.h"

struct FSensorCapturePool
{    
    inline static TMap<AActor*, TArray<USceneCaptureComponent2D*>> PoolCaptures;

    static USceneCaptureComponent2D* GetFirst(AActor* actor)
    {
        TArray<USceneCaptureComponent2D*>* captures = PoolCaptures.Find(actor);
        if (captures == nullptr || captures->Num() <= 0)
        {
            return nullptr;
        }
        
        return captures->Top();
    }

    static bool AddCapture(AActor* actor, USceneCaptureComponent2D* capture)
    {
        TArray<USceneCaptureComponent2D*>* captures = PoolCaptures.Find(actor);
        if (captures == nullptr)
        {
            PoolCaptures.Add(actor, TArray<USceneCaptureComponent2D*>());
            captures = &PoolCaptures[actor];
        }
        int num = captures->Num(); 
        captures->AddUnique(capture);
        bool badded = num != captures->Num();
        //if (badded == true)
        //{
        //    capture->bCaptureEveryFrame = false;
        //}
        return badded;
    }
    
    static bool RemoveCapture(AActor* actor, USceneCaptureComponent2D* capture)
    {
        TArray<USceneCaptureComponent2D*>* captures = PoolCaptures.Find(actor);
        if (captures == nullptr)
        {
            return false;
        }
        int num = captures->Num(); 
        captures->Remove(capture);
        return num != captures->Num();
    }

    static int32 RemoveCaptureAll(AActor* actor)
    {
        return PoolCaptures.Remove(actor);
    }
    
    static bool ActivateCaptures(AActor* actor, bool bActive)
    {
        TArray<USceneCaptureComponent2D*>* captures = PoolCaptures.Find(actor);
        if (captures == nullptr)
        {
            return false;
        }

        //bool bAny = false;
        //for (int i = 0; i < captures->Num(); ++i)
        //{
        //    USceneCaptureComponent2D* capture = (*captures)[i];
        //    capture->bCaptureEveryFrame = bActive;
        //    bAny = true;
        //}
        //return bAny;
        return true;
    }

    static void Clear()
    {
        PoolCaptures.Empty();
    }
};
