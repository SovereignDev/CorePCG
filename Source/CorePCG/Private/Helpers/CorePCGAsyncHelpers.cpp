// © 2021, Brock Marsh. All rights reserved.

#include "Helpers/CorePCGAsyncHelpers.h"

#include "PCGComponent.h"
#include "Data/PCGPointData.h"
#include "Helpers/CorePCGLibrary.h"
#include "Helpers/CorePCGStatics.h"

FPCGContext* FCorePCGAsyncElementBase::Initialize(const FPCGDataCollection& InputData, TWeakObjectPtr<UPCGComponent> SourceComponent, const UPCGNode* Node)
{
	FCorePCGAsyncContext* Context = new FCorePCGAsyncContext();
	Context->InputData = InputData;
	Context->SourceComponent = SourceComponent;
	Context->Node = Node;

	return Context;
}

bool FCorePCGAsyncElementBase::ExecuteInternal(FPCGContext* Context) const
{
	FCorePCGAsyncContext* AsyncContext = static_cast<FCorePCGAsyncContext*>(Context);

	// Cancel the Async Operation if requested
	if(AsyncContext->IsCanceled()) return true;

	// Can only hit here once FinishExecute() has been called
	if(AsyncContext->HasAsyncExecutionBegan())
	{
		AsyncContext->Reset();
		return true;
	}

	// Pause the execution until FinishExecute() is called
	AsyncContext->Begin();

	// Call the Wrapped Execute Function
	bool bSuccess = AsyncExecuteInternal(AsyncContext);

	// If Failed, then Cancel the Async Operation
	if(!bSuccess)
	{
		AsyncContext->Cancel();
		return true;
	}

	// Return false so PCG doesnt continue executing
	return false;
}

void FCorePCGAsyncElementBase::FinishExecute(FPCGContext* Context)
{
	// UnPause Execution. This will cause ExecuteInternal() to return true
	Context->bIsPaused = false;
}

void FCorePCGAsyncElementBase::CancelExecute(FCorePCGAsyncContext* Context)
{
	Context->Cancel();
}


FPCGContext* FCorePCGPointProcessingAsyncElementBase::Initialize(const FPCGDataCollection& InputData, TWeakObjectPtr<UPCGComponent> SourceComponent, const UPCGNode* Node)
{
	FCorePCGAsyncContext* Context = new FCorePCGAsyncContext();
	Context->InputData = InputData;
	Context->SourceComponent = SourceComponent;
	Context->Node = Node;

	return Context;
}

bool FCorePCGPointProcessingAsyncElementBase::ExecuteInternal(FPCGContext* Context) const
{
	FCorePCGAsyncContext* AsyncContext = static_cast<FCorePCGAsyncContext*>(Context);

	// Cancel the Async Operation if requested
	if(AsyncContext->IsCanceled()) return true;
	
	// Can only hit here once FinishExecute() has been called
	if(AsyncContext->HasAsyncExecutionBegan())
	{
		AsyncContext->Reset();
		return true;
	}

	// Pause the execution until FinishExecute() is called
	AsyncContext->Begin();

	// Call the Wrapped Execute Function
	bool bSuccess = AsyncExecuteInternal(AsyncContext);

	// If Failed, then Cancel the Async Operation
	if(!bSuccess)
	{
		Context->bIsPaused = false;
		return true;
	}

	// Return false so PCG doesnt continue executing
	return false;
}

void FCorePCGPointProcessingAsyncElementBase::FinishExecute(FPCGContext* Context)
{
	// UnPause Execution. This will cause ExecuteInternal() to return true
	Context->bIsPaused = false;
}

void FCorePCGAsyncIterationElementBase::ProcessPointsIterationConstant(FCorePCGAsyncContext* Context, int32 MaxIterationsPerTick, const TFunction<bool(int32, FPCGPoint&)>& Function, const TFunction<void()>& OnFinishedFunction) const
{
	if(!PCG::HasAnyInputs(Context)) return;
	
	UCorePCGLibrary::MergeInputData(Context);
	
	TArray<FPCGTaggedData> Inputs = Context->InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel);
	
	const UPCGPointData* PointData = PCG::AsPointData(Context, Inputs[0].Data);
	if (!PointData) return;

	// Dummy Array to pass into the ProcessPointsIteration function
	TArray<FPCGPoint> NewPoints;

	ProcessPointsIteration(Context, PointData->GetPoints(), NewPoints, MaxIterationsPerTick, Function, OnFinishedFunction);
}

void FCorePCGAsyncIterationElementBase::ProcessPointsIteration(FCorePCGAsyncContext* Context, int32 MaxIterationsPerTick, const TFunction<bool(int32, FPCGPoint&)>& Function, const TFunction<void()>& OnFinishedFunction) const
{
	if(!PCG::HasAnyInputs(Context)) return;
	
	UCorePCGLibrary::MergeInputData(Context);
	
	TArray<FPCGTaggedData> Inputs = Context->InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel);
	TArray<FPCGTaggedData>& Outputs = Context->OutputData.TaggedData;
	
	Outputs.Reserve(Inputs.Num());

	FPCGTaggedData MergedInput = Inputs[0];
	
	const UPCGPointData* PointData = PCG::AsPointData(Context, MergedInput.Data);
	if (!PointData) return;
	
	UPCGPointData* NewData = NewObject<UPCGPointData>();
	NewData->InitializeFromData(PointData);
	Outputs.Add(FPCGTaggedData(NewData, MergedInput.Tags, MergedInput.Pin));

	TArray<FPCGPoint>& NewPoints = NewData->GetMutablePoints();

	ProcessPointsIteration(Context, PointData->GetPoints(), NewPoints, MaxIterationsPerTick, Function, OnFinishedFunction);
}

void FCorePCGAsyncIterationElementBase::ProcessPointsIteration(FCorePCGAsyncContext* Context, const TArray<FPCGPoint>& InPoints, TArray<FPCGPoint>& OutPoints, int32 MaxIterationsPerTick, const TFunction<bool(int32, FPCGPoint&)>& Function, const TFunction<void()>& OnFinishedFunction) const
{
	constexpr int32 CurrentIteration = 0;

	if(InPoints.IsEmpty()) return;
	
	if(!OutPoints.IsValidIndex(0)) OutPoints.Reserve(InPoints.Num());
	
	_Internal_Iteration(Context, InPoints, OutPoints, MaxIterationsPerTick, CurrentIteration, Function, OnFinishedFunction);
}

void FCorePCGAsyncIterationElementBase::_Internal_Iteration(FCorePCGAsyncContext* Context, const TArray<FPCGPoint>& InPoints, TArray<FPCGPoint>& OutPoints, const int32 MaxIterationsPerTick, const int32 CurrentIteration, const TFunction<bool(int32, FPCGPoint&)>& Function, const TFunction<void()>& OnFinishedFunction) const
{
	// If Source Component is invalid, then the PCG was probably destroyed, so cancel the async operation
	if(!Context->SourceComponent.IsValid()) Context->Cancel();

	// If we have been cancelled, then return without doing anything
	if(Context->IsCanceled()) return;
	
	for(int32 Index = CurrentIteration * MaxIterationsPerTick; Index < (CurrentIteration + 1) * MaxIterationsPerTick; ++Index)
	{
		// If we have reached the end of the points, then call the OnFinishedFunction
		if(Index >= InPoints.Num())
		{
			OnFinishedFunction();
			return;
		}

		FPCGPoint Point = InPoints[Index];

		const bool bValidPoint = Function(Index, Point);

		// If false then we want to remove the point from the Array
		if(bValidPoint)
		{
			OutPoints.Add(Point);
		}
	}

	_Internal_TickIteration(Context, InPoints, OutPoints, MaxIterationsPerTick, CurrentIteration + 1, Function, OnFinishedFunction);
}

void FCorePCGAsyncIterationElementBase::_Internal_TickIteration(FCorePCGAsyncContext* Context, const TArray<FPCGPoint>& InPoints, TArray<FPCGPoint>& OutPoints, int32 MaxIterationsPerTick, int32 CurrentIteration, const TFunction<bool(int32, FPCGPoint&)>& Function, const TFunction<void()>& OnFinishedFunction) const
{
	// If Source Component is invalid, then the PCG was probably destroyed, so cancel the async operation
	if(!Context->SourceComponent.IsValid() || !Context->SourceComponent.Get()->GetWorld()) Context->Cancel();

	// If we have been cancelled, then dont begin the next iteration
	if(Context->IsCanceled()) return;
	
	Context->SourceComponent.Get()->GetWorld()->GetTimerManager().SetTimerForNextTick([this, Context, &InPoints, &OutPoints, MaxIterationsPerTick, CurrentIteration, Function, OnFinishedFunction]()
	{
		_Internal_Iteration(Context, InPoints, OutPoints, MaxIterationsPerTick, CurrentIteration, Function, OnFinishedFunction);
	});
}
