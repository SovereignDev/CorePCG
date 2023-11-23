// © 2021, Brock Marsh. All rights reserved.

#include "Nodes/CorePCG_AsyncTest.h"

#include "PCGComponent.h"
#include "PCGContext.h"
#include "Data/PCGPointData.h"
#include "Data/PCGSpatialData.h"
#include "Helpers/PCGAsync.h"

FPCGContext* FPCGAsyncTestElement::Initialize(const FPCGDataCollection& InputData, TWeakObjectPtr<UPCGComponent> SourceComponent, const UPCGNode* Node)
{
	FPCGAsyncTestContext* Context = new FPCGAsyncTestContext();
	Context->InputData = InputData;
	Context->SourceComponent = SourceComponent;
	Context->Node = Node;

	return Context;
}

bool FPCGAsyncTestElement::ExecuteInternal(FPCGContext* InContext) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FPCGAsyncTestElement::Execute);

	FPCGAsyncTestContext* Context = static_cast<FPCGAsyncTestContext*>(InContext);
	
	const UPCGAsyncTestSettings* Settings = Context->GetInputSettings<UPCGAsyncTestSettings>();
	check(Settings);
	TArray<FPCGTaggedData> Inputs = Context->InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel);
	TArray<FPCGTaggedData>& Outputs = Context->OutputData.TaggedData;

	if(Inputs.IsValidIndex(0)) Outputs.Emplace_GetRef().Data = Inputs[0].Data;
	
	if(Context->TimerHandle.IsValid()) return true;
	
	Context->bIsPaused = true;

	UE_LOG(LogTemp, Error, TEXT("Starting Async Test"))

	auto Lambda = [Context]()
	{
		UE_LOG(LogTemp, Error, TEXT("Async Timer has Finished"))
		
		Context->bIsPaused = false;
	};

	Context->SourceComponent.Get()->GetWorld()->GetTimerManager().SetTimer(Context->TimerHandle, Lambda, 5.0f, false);

	return false;
}

bool FPCGAsyncTestElement2::AsyncExecuteInternal(FCorePCGAsyncContext* Context) const
{
	const UPCGAsyncTestSettings* Settings = Context->GetInputSettings<UPCGAsyncTestSettings>();
	check(Settings);
	
	ProcessPointsIteration(Context, 10, [](int32 Index, FPCGPoint& Point)
	{
		Point.Seed = 69420;
		
		UE_LOG(LogTemp, Error, TEXT("Iteration: %d | Frame: %d"), Index, (int32)GFrameCounter);
		
		return true;
	}, FinishExecuteFunction(Context));
	
	return true;
}