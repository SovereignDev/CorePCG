// © 2021, Brock Marsh. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "PCGContext.h"
#include "Elements/PCGPointProcessingElementBase.h"
#include "Engine/AssetManager.h"

/**
 *	
 */
struct COREPCG_API FCorePCGAsyncContext : public FPCGContext
{
public:
	void Begin() { bHasAsyncExecuted = bIsPaused = true; }
	void Cancel() { bCancelAsync = true; bIsPaused = false; }

	bool bCancelAsync = false;
	bool bHasAsyncExecuted = false;
};

class COREPCG_API FCorePCGAsyncElementBase : public FSimplePCGElement
{
protected:
	virtual FPCGContext* Initialize(const FPCGDataCollection& InputData, TWeakObjectPtr<UPCGComponent> SourceComponent, const UPCGNode* Node) override;

	virtual bool ExecuteInternal(FPCGContext* Context) const override final;
	
	virtual bool AsyncExecuteInternal(FCorePCGAsyncContext* Context) const {return false;}

	// Call this once the async operation has finished
	static void FinishExecute(FPCGContext* Context);
	// Call this to cancel the async operation. This will also FinishExecute()
	static void CancelExecute(FCorePCGAsyncContext* Context);

	static TFunction<void()> FinishExecuteFunction(FPCGContext* Context) { return [Context]{FinishExecute(Context);}; }
};

class COREPCG_API FCorePCGPointProcessingAsyncElementBase : public FPCGPointProcessingElementBase
{
protected:
	virtual FPCGContext* Initialize(const FPCGDataCollection& InputData, TWeakObjectPtr<UPCGComponent> SourceComponent, const UPCGNode* Node) override;

	virtual bool ExecuteInternal(FPCGContext* Context) const override final;
	
	virtual bool AsyncExecuteInternal(FCorePCGAsyncContext* Context) const {return false;}

	// Call this once the async operation has finished
	static void FinishExecute(FPCGContext* Context);
};

/*
 *	Base Class for Performing a Point Loop over time.
 */
class COREPCG_API FCorePCGAsyncIterationElementBase : public FCorePCGAsyncElementBase
{
protected:
	virtual bool IsCacheable(const UPCGSettings* InSettings) const override { return false; }

	/* Point Loop but splits the iterations up over time based on the MaxIterationPerTick. This Iterates over all Inputs automatically and sets the Content->Outputs. NOTE: This will Merge the Input data */
	void ProcessPointsIterationConstant(FCorePCGAsyncContext* Context, int32 MaxIterationsPerTick, const TFunction<bool(int32, FPCGPoint&)>& Function, const TFunction<void()>& OnFinishedFunction = [](){}) const;

	/* Point Loop but splits the iterations up over time based on the MaxIterationPerTick. This Iterates over all Inputs automatically and sets the Content->Outputs. NOTE: This will Merge the Input data */
	void ProcessPointsIteration(FCorePCGAsyncContext* Context, int32 MaxIterationsPerTick, const TFunction<bool(int32, FPCGPoint&)>& Function, const TFunction<void()>& OnFinishedFunction = [](){}) const;

	/* Point Loop but splits the iterations up over time based on the MaxIterationPerTick */
	void ProcessPointsIteration(FCorePCGAsyncContext* Context, const TArray<FPCGPoint>& InPoints, TArray<FPCGPoint>& OutPoints, int32 MaxIterationsPerTick, const TFunction<bool(int32, FPCGPoint&)>& Function, const TFunction<void()>& OnFinishedFunction = [](){}) const;

private:
	void _Internal_Iteration(FCorePCGAsyncContext* Context, const TArray<FPCGPoint>& InPoints, TArray<FPCGPoint>& OutPoints, int32 MaxIterationsPerTick, int32 CurrentIteration, const TFunction<bool(int32, FPCGPoint&)>& Function, const TFunction<void()>& OnFinishedFunction) const;
	void _Internal_TickIteration(FCorePCGAsyncContext* Context, const TArray<FPCGPoint>& InPoints, TArray<FPCGPoint>& OutPoints, int32 MaxIterationsPerTick, int32 CurrentIteration, const TFunction<bool(int32, FPCGPoint&)>& Function, const TFunction<void()>& OnFinishedFunction) const;
};


namespace CorePCGAsyncLoadHelpers
{
	/* Wrapper for the Streamable Manager Async Load that will call the delegate immediately if the object is already loaded */
	template<class T = UObject>
	static TSharedPtr<struct FStreamableHandle> RequestAsyncLoad(const TSoftObjectPtr<T>& AssetToLoad, const FStreamableDelegate& DelegateToCall = FStreamableDelegate())
	{
		if(AssetToLoad.IsNull()) return nullptr;

		if(AssetToLoad.IsValid())
		{
			DelegateToCall.ExecuteIfBound();
			return nullptr;
		}

		return UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(AssetToLoad.ToSoftObjectPath(), DelegateToCall);
	}

	// Wrapper for Async Loading a TSoftObjectPtr Array
	template<class T = UObject>
	FORCEINLINE static TSharedPtr<struct FStreamableHandle> RequestAsyncLoad(const TArray<TSoftObjectPtr<T>>& AssetsToLoad, const FStreamableDelegate& DelegateToCall = FStreamableDelegate())
	{
		if(AssetsToLoad.IsEmpty()) return nullptr;
	
		return UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(SoftPointersToPaths<T>(AssetsToLoad), DelegateToCall);
	}

	// Wrapper for Async Loading a TSoftClassPtr Array
	template<class T = UObject>
	FORCEINLINE static TSharedPtr<struct FStreamableHandle> RequestAsyncLoad(const TArray<TSoftClassPtr<T>>& AssetsToLoad, const FStreamableDelegate& DelegateToCall = FStreamableDelegate())
	{
		if(AssetsToLoad.IsEmpty()) return nullptr;
	
		return UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(SoftPointersToPaths<T>(AssetsToLoad), DelegateToCall);
	}

	// Converts a TSoftObjectPtr Array to a FSoftObjectPath Array
	template<typename T = UObject>
	FORCEINLINE static TArray<FSoftObjectPath> SoftPointersToPaths(const TArray<TSoftObjectPtr<T>>& SoftObjectPtrs)
	{
		TArray<FSoftObjectPath> Out;
		Out.Reserve(SoftObjectPtrs.Num());

		for (const TSoftObjectPtr<T>& SoftObjectPtr : SoftObjectPtrs) Out.Add(SoftObjectPtr.ToSoftObjectPath());

		return Out;
	}
	// Converts a TSoftClassPtr to a FSoftObjectPath
	template<typename T = UObject>
	FORCEINLINE static TArray<FSoftObjectPath> SoftPointersToPaths(const TSoftObjectPtr<T>& SoftObjectPtrs)
	{
		return {SoftObjectPtrs.ToSoftObjectPath()};
	}

	// Converts a TSoftClassPtr Array to a FSoftObjectPath Array
	template<typename T = UObject>
	FORCEINLINE static TArray<FSoftObjectPath> SoftPointersToPaths(const TArray<TSoftClassPtr<T>>& SoftObjectPtrs)
	{
		TArray<FSoftObjectPath> Out;
		Out.Reserve(SoftObjectPtrs.Num());

		for (const TSoftClassPtr<T>& SoftObjectPtr : SoftObjectPtrs) Out.Add(SoftObjectPtr.ToSoftObjectPath());

		return Out;
	}
	template<typename T = UObject>
	FORCEINLINE static TArray<FSoftObjectPath> SoftPointersToPaths(const TSoftClassPtr<T>& SoftObjectPtrs)
	{
		return {SoftObjectPtrs.ToSoftObjectPath()};
	}
}