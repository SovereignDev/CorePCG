// © 2021, Brock Marsh. All rights reserved.


#include "Nodes/CorePCG_SampleSplineEnds.h"
#include "PCGContext.h"
#include "Data/PCGPointData.h"
#include "Data/PCGPolyLineData.h"
#include "Helpers/PCGHelpers.h"

bool FPCGSampleSplineEndsElement::ExecuteInternal(FPCGContext* Context) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FPCGSampleSplineEndsElement::Execute);
	
	const UPCGSampleSplineEndsSettings* Settings = Context->GetInputSettings<UPCGSampleSplineEndsSettings>();
	
	check(Settings);
	
	TArray<FPCGTaggedData> Inputs = Context->InputData.GetInputsByPin("Spline");
	TArray<FPCGTaggedData>& Outputs = Context->OutputData.TaggedData;

	if(Inputs.IsEmpty() || !Inputs[0].Data) return true;
	
	UPCGPolyLineData* SplineData = Cast<UPCGPolyLineData>(Inputs[0].Data);
	
	if(!SplineData)
	{
		PCGE_LOG(Error, GraphAndLog, FText::FromString("Input is not valid Spline Data"));
		return true;
	}

	UPCGPointData* SampledPointData = NewObject<UPCGPointData>();
	SampledPointData->InitializeFromData(SplineData);

	TArray<FPCGPoint> OutPoints;
	OutPoints.Reserve(Settings->bSampleSplineStart + Settings->bSampleSplineEnd);
	
	if(Settings->bSampleSplineStart)
	{
		FTransform StartTransform = SplineData->GetTransformAtDistance(0, 0, true);
		
		FPCGPoint StartPoint(StartTransform, 1, PCGHelpers::ComputeSeed((int)StartTransform.GetLocation().X, (int)StartTransform.GetLocation().Y, (int)StartTransform.GetLocation().Z));

		OutPoints.Add(StartPoint);
	}

	if(Settings->bSampleSplineEnd)
	{
		int Index = SplineData->GetNumSegments() - 1;
		
		FTransform StartTransform = SplineData->GetTransformAtDistance(Index, SplineData->GetSegmentLength(Index), true);
		
		FPCGPoint StartPoint(StartTransform, 1, PCGHelpers::ComputeSeed((int)StartTransform.GetLocation().X, (int)StartTransform.GetLocation().Y, (int)StartTransform.GetLocation().Z));

		OutPoints.Add(StartPoint);
	}

	SampledPointData->SetPoints(OutPoints);

	FPCGTaggedData& Output = Outputs.Emplace_GetRef();
	Output.Data = SampledPointData;
	Output.Pin = "Out";

	return true;
}

FText UPCGSampleSplineEndsSettings::GetNodeTooltipText() const
{
	return NSLOCTEXT("PCGSampleSplineEndsSettings", "NodeTooltip", "Creates a point for the Start and End of a Spline");
}
