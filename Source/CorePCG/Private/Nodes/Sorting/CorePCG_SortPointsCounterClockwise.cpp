// © 2021, Brock Marsh. All rights reserved.


#include "Nodes/Sorting/CorePCG_SortPointsCounterClockwise.h"

#include "PCGContext.h"
#include "Helpers/CorePCGStatics.h"

bool FPCGSortPointsCounterClockwiseElement::ExecuteInternal(FPCGContext* Context) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FPCGSortPointsCounterClockwiseElement::Execute);

	if(!PCG::HasAnyInputs(Context)) return true;
	
	const UPCGSortPointsCounterClockwiseSettings* Settings = Context->GetInputSettings<UPCGSortPointsCounterClockwiseSettings>();
	check(Settings);

	TArray<FPCGTaggedData> Inputs = Context->InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel);
	TArray<FPCGTaggedData>& Outputs = Context->OutputData.TaggedData;;
	
	for(const FPCGTaggedData& Input : Inputs)
	{
		const UPCGSpatialData* SpatialData = Cast<UPCGSpatialData>(Input.Data);
		if(!SpatialData) continue;
		const UPCGPointData* PointData = SpatialData->ToPointData(Context);
		if(!PointData) continue;
		
		UPCGPointData* NewData = NewObject<UPCGPointData>();
		NewData->InitializeFromData(PointData);

		TArray<FPCGPoint> Points = PointData->GetPoints();

		// Find Center of Points
		FVector Center = FVector::ZeroVector;
		for (const FPCGPoint& Point : Points)
		{
			Center += Point.Transform.GetLocation();
		}
		Center /= Points.Num();
		
		// Sort Points Counter Clockwise
		Points.Sort([Center](const FPCGPoint& A, const FPCGPoint& B)
		{
			return FMath::Atan2((A.Transform.GetLocation() - Center).Y, (A.Transform.GetLocation() - Center).X) > FMath::Atan2((B.Transform.GetLocation() - Center).Y, (B.Transform.GetLocation() - Center).X);
		});

		NewData->SetPoints(Points);

		Outputs.Emplace(FPCGTaggedData(NewData, {}, PCGPinConstants::DefaultOutputLabel));
	}

	return true;
}