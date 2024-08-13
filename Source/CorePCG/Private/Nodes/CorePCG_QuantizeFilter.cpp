// © 2021, Brock Marsh. All rights reserved.


#include "Nodes\CorePCG_QuantizeFilter.h"

#include "PCGContext.h"
#include "Helpers/CorePCGStatics.h"
#include "Helpers/PCGHelpers.h"

bool FPCGQuantizeFilterElement::AsyncExecuteInternal(FCorePCGAsyncContext* Context) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FPCGQuantizeFilterElement::Execute);

	if (!PCG::HasAnyInputs(Context)) return false;

	const UPCGQuantizeFilterSettings* Settings = Context->GetInputSettings<UPCGQuantizeFilterSettings>();
	check(Settings);

	float InCellSize = Settings->CellSize;
	float OverlapThreshold = Settings->OverlapThreshold;
	int32 Seed = Context->GetSeed();
	
	AsyncTask(ENamedThreads::AnyThread, [Context, InCellSize, Seed, OverlapThreshold]
	{
	    TArray<FPCGTaggedData> Inputs = Context->InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel);
	    TArray<FPCGTaggedData>& Outputs = Context->OutputData.TaggedData;

		UPCGSpatialData* InputData = const_cast<UPCGSpatialData*>(Cast<UPCGSpatialData>(Inputs[0].Data));
		
		if(!InputData)
		{
			FinishExecute(Context);
			return;
		}

		float CellSize = InCellSize * (OverlapThreshold + 1.f);
		
	    // First Build the Spatial Hash Grid of Points based on Cell Size
	    TMap<FIntPoint, TArray<FPCGPoint>> PointGrid;

	    PCG::ConstantProcessPointsSynchronous(Context, Inputs, [&PointGrid, CellSize](const FPCGPoint& Point)
	    {
			const FVector Location = Point.Transform.GetLocation();
			const FIntPoint Key = FIntPoint(Location.X / CellSize, Location.Y / CellSize);

			if (!PointGrid.Contains(Key)) PointGrid.Add(Key, TArray<FPCGPoint>());

			PointGrid[Key].Add(Point);
	    });

	    // The Points that will be returned
	    TArray<FPCGPoint> Points;
	    Points.Reserve(PointGrid.Num());

		const float OverlapDistanceSquared = FMath::Square(CellSize);

		FRandomStream Stream(Seed);

	    // Get Random point from each Partition Cell
	    for (TTuple<FIntPoint, TArray<FPCGPoint>>& Cell : PointGrid)
	    {
			// For Each Cell find a Random Point that doesnt overlap any previous points
			bool bFoundPoint = false;

			while (!bFoundPoint)
			{
			    if(Cell.Value.IsEmpty()) break;
				
			    const int32 RandomIndex = Stream.RandRange(0, Cell.Value.Num() - 1);
			    FPCGPoint RandomPoint = Cell.Value[RandomIndex];

			    bFoundPoint = true;

			    // Make Sure we dont overlap any already generated points
			    for (const FPCGPoint& Point : Points)
			    {
			    	FVector PointLocation = Point.Transform.GetLocation();
			    	FVector RandomPointLocation = RandomPoint.Transform.GetLocation();
			    	
					if (FVector::DistSquared(PointLocation, RandomPointLocation) <= OverlapDistanceSquared)
					{
					    bFoundPoint = false;
					    break;
					}
			    }

			    if(bFoundPoint)
			    {
					Points.Add(RandomPoint);
					break;
			    }
			    else
			    {
					Cell.Value.RemoveAtSwap(RandomIndex);
			    }
			}
	    }

		UPCGPointData* OutputData = NewObject<UPCGPointData>();
		OutputData->InitializeFromData(InputData);
		OutputData->SetPoints(Points);

		if(Outputs.IsValidIndex(0)) Outputs[0].Data = OutputData;
		else Outputs.Add(FPCGTaggedData(OutputData));

		FinishExecute(Context);
	});

	return true;
}
