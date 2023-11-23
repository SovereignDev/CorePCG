// © 2021, Brock Marsh. All rights reserved.


#include "Nodes/CorePCG_AssignTag.h"

#include "PCGContext.h"

bool FPCGAssignTagElement::ExecuteInternal(FPCGContext* Context) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FPCGAssignTagElement::Execute);
	
	const UPCGAssignTagSettings* Settings = Context->GetInputSettings<UPCGAssignTagSettings>();
	check(Settings);
	
	TArray<FPCGTaggedData> Inputs = Context->InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel);
	TArray<FPCGTaggedData>& Outputs = Context->OutputData.TaggedData;

	for (FPCGTaggedData& Input : Inputs)
	{
		Input.Tags.Append(Settings->Tags);
	}

	Outputs = Inputs;
	
	return true;
}

UPCGAssignTagSettings::UPCGAssignTagSettings()
{
#if WITH_EDITOR
	Category = FText::FromString("Filter");
#endif
}
