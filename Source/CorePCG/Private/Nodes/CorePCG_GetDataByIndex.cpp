// © 2021, Brock Marsh. All rights reserved.


#include "Nodes/CorePCG_GetDataByIndex.h"

#include "PCGContext.h"
#include "Helpers/CorePCGStatics.h"

bool FPCGGetDataByIndexElement::ExecuteInternal(FPCGContext* Context) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FPCGGetDataByIndexElement::Execute);

	if(!PCG::HasAnyInputs(Context)) return true;
	
	const UPCGGetDataByIndexSettings* Settings = Context->GetInputSettings<UPCGGetDataByIndexSettings>();
	check(Settings);
	
	TArray<FPCGTaggedData> Inputs = Context->InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel);
	TArray<FPCGTaggedData>& Outputs = Context->OutputData.TaggedData;

	const int32 Index = Settings->Index;

	if(Inputs.IsValidIndex(Index))
	{
		Outputs.Emplace(Inputs[Index]);
	}
	else
	{
		PCGE_LOG(Error, GraphAndLog, FText::FromString("Cannot get data by index, index is out of range"));
	}
	
	return true;
}