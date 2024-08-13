// © 2021, Brock Marsh. All rights reserved.


#include "Nodes/CorePCG_DensityFromNeighbors.h"

#include "PCGContext.h"
#include "Data/PCGPointData.h"
#include "Helpers/CorePCGLibrary.h"
#include "Helpers/CorePCGStatics.h"
#include "Helpers/PCGAsync.h"

const UPCGPointData* GetPointData(FPCGContext* Context, const FPCGTaggedData& In)
{
	const UPCGSpatialData* SData = Cast<UPCGSpatialData>(In.Data);
	
	if(!SData) return nullptr;

	return SData->ToPointData(Context);
}

bool FPCGDensityFromNeighborsElement::AsyncExecuteInternal(FCorePCGAsyncContext* Context) const
{
	const UPCGDensityFromNeighborsSettings* Settings = Context->GetInputSettings<UPCGDensityFromNeighborsSettings>();
	check(Settings);
	
	if(!PCG::HasAnyInputs(Context)) return true;

	float Radius = Settings->Radius;
	bool b2D = Settings->bCalculate2DSpace;
	bool bHasConstantSize = Settings->bHasConstantSize;

	bool bLogTime =
#if WITH_EDITORONLY_DATA
		Settings->bLogTime;
#else
			false;
#endif
	
	Async(EAsyncExecution::ThreadPool, [this, Context, Radius, b2D, bHasConstantSize, bLogTime]()
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FPCGDensityFromNeighborsElement::Execute);

		double RawStartTime = FPlatformTime::Seconds();
		double StartTime = FPlatformTime::Seconds();

		const TArray<FPCGTaggedData>& Inputs = Context->InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel);

		// With bHasConstantSize we can do a much faster calculation. O(n * Floor(Radius % PointRadius)^2-1) i.e if Radius = 200, PointRadius = 100 : O(n * 2^2-1) = O(n * 3)
		// instead of O(n^2)
		if(bHasConstantSize)
		{
			const UPCGPointData* FirstPointData = Cast<UPCGPointData>(Inputs[0].Data);
			if(!FirstPointData) return false;
		
			// First we need to find the Max Neighbor Count so we can normalize the Density.
			double PointRadius = FirstPointData->GetPoint(0).GetScaledExtents().GetMax();

			// Calculate the Max Number of Points that can fit inside a radius using the constant Point Radius.
			int32 MaxNeighbors = FMath::Floor(Radius / PointRadius);
			
			/* Async Create the Spatial Grid Hash */
			// Function that will return the Spatial Grid Hash of all the input points
			auto AsyncPointGrid = Async(EAsyncExecution::ThreadPool,[Context, &Inputs, PointRadius]
			{
				TMap<FIntPoint, FPCGPoint> PointGrid;

				// Add all Points to the Grid
				PCG::ConstantProcessPointsSynchronous(Context, Inputs, [&PointGrid, PointRadius](const FPCGPoint& InPoint)->bool
				{
					const FIntPoint Coords = FIntPoint(FMath::Floor(InPoint.Transform.GetLocation().X / (PointRadius * 2)), FMath::Floor(InPoint.Transform.GetLocation().Y / (PointRadius * 2)));
	
					FPCGPoint CopyPoint = InPoint;
					CopyPoint.Density = 0;
				
					PointGrid.Add(Coords, CopyPoint);
				
					return true;
				});

				return PointGrid;
			});
		
			// Wait for the Async to finish
			AsyncPointGrid.Wait();

			// Store the Result so we can modify it.
			TMap<FIntPoint, FPCGPoint> PointGrid = AsyncPointGrid.Get();

			if(bLogTime) UE_LOG(LogPCG, Warning, TEXT("Async Grid Creation took %f ms to execute"), (FPlatformTime::Seconds() - StartTime) * 1000);
			StartTime = FPlatformTime::Seconds();

			int32 MaxOverlaps = 0;

			// Find Overlapping Neighbors and store in the Density
			for (TTuple<FIntPoint, FPCGPoint>& Grid : PointGrid)
			{
				FIntPoint CurrentCell = Grid.Key;
			
				for(int32 X = CurrentCell.X - MaxNeighbors; X <= CurrentCell.X + MaxNeighbors; ++X)
				{
					for(int32 Y = CurrentCell.Y - MaxNeighbors; Y <= CurrentCell.Y + MaxNeighbors; ++Y)
					{
						// Dont Check the Current Cell
						if(X == CurrentCell.X && Y == CurrentCell.Y) continue;

						const FIntPoint OtherCell = FIntPoint(X, Y);
	
						if(!PointGrid.Contains(OtherCell)) continue;
					
						FPCGPoint OtherPoint = PointGrid[OtherCell];
					
						FVector CurrentLocation = Grid.Value.Transform.GetLocation();
						FVector OtherLocation = OtherPoint.Transform.GetLocation();
	
						if(b2D)
						{
							CurrentLocation.Z = 0;
							OtherLocation.Z = 0;
						}
	
						FBox OtherBox = OtherPoint.GetLocalBounds().TransformBy(FTransform(OtherLocation));

						if(FMath::SphereAABBIntersection(CurrentLocation, FMath::Square(Radius) + FMath::Square(PointRadius), OtherBox))
						{
							Grid.Value.Density++;
						}
					}
				}
				
				MaxOverlaps = FMath::Max(MaxOverlaps, (int32)Grid.Value.Density);
			}
			
			if(bLogTime) UE_LOG(LogPCG, Warning, TEXT("Point Processing took %f ms to execute"), (FPlatformTime::Seconds() - StartTime) * 1000);
			StartTime = FPlatformTime::Seconds();

			TArray<FPCGPoint> OutPoints;
			PointGrid.GenerateValueArray(OutPoints);
			
			// Normalize the Density of all points
			for (FPCGPoint& Point : OutPoints)
			{
				Point.Density /= MaxOverlaps;
			}
			
			if(bLogTime) UE_LOG(LogPCG, Warning, TEXT("Normalization took %f ms to execute"), (FPlatformTime::Seconds() - StartTime) * 1000);

			// If Invalid then this was cancelled on the Game Thread
			if(!PCG::IsDataValidOnAnyThread(FirstPointData)) return false;
			if(!Context) return false;

			UPCGPointData* OutData = NewObject<UPCGPointData>();
			OutData->InitializeFromData(FirstPointData);
			OutData->SetPoints(OutPoints);
			
			if(Context->OutputData.TaggedData.IsValidIndex(0)) Context->OutputData.TaggedData[0].Data = OutData;
			else Context->OutputData.TaggedData.Add(FPCGTaggedData(OutData));
		
			if(bLogTime) UE_LOG(LogPCG, Warning, TEXT("Total function took %f ms to execute"), (FPlatformTime::Seconds() - RawStartTime) * 1000);
			
			return true;
		}
		else
		{
			float Diameter = Radius * 2;
			int MaxNeighbors = 0;
		
			// Horrible O(n^2) nested loop to find the max neighbor count.
			for (const FPCGTaggedData& Input : Inputs)
			{
				const UPCGPointData* PointData = GetPointData(Context, Input);

				if(!PointData) { PCGE_LOG(Warning, GraphAndLog, FText::FromString("Input Data is not a Point or Spatial Data")); continue; }

				for (FPCGPoint CurrentPoint : PointData->GetPoints())
				{
					int32 NeighborCount = 0;
			
					for (FPCGTaggedData NestedInput : Inputs)
					{
						const UPCGPointData* NestedPointData = GetPointData(Context, Input);
				
						if(!NestedPointData) { PCGE_LOG(Warning, GraphAndLog, FText::FromString("Input Data is not a Point or Spatial Data")); continue; }

						for (FPCGPoint OtherPoint : NestedPointData->GetPoints())
						{
							FVector CurrentLocation = CurrentPoint.Transform.GetLocation();
							FVector OtherLocation = OtherPoint.Transform.GetLocation();

							if(b2D)
							{
								CurrentLocation.Z = 0;
								OtherLocation.Z = 0;
							}
					
							if((CurrentLocation - OtherLocation).Length() <= Diameter)
							{
								NeighborCount++;
							}
						}
					}

					MaxNeighbors = FMath::Max(MaxNeighbors, NeighborCount);
				}
			}
			
			/*
			UCorePCGLibrary::MergeInputData(Context);
			
			Outputs.Reserve(Inputs.Num());
		
			FPCGTaggedData MergedInput = Inputs[0];
			
			const UPCGPointData* PointData = PCG::AsPointData(Context, MergedInput.Data);
			if (!PointData) return false;
		
			const TArray<FPCGPoint>& InputPoints = PointData->GetPoints();
			
			UPCGPointData* NewData = NewObject<UPCGPointData>();
			NewData->InitializeFromData(PointData);
			Outputs.Add(FPCGTaggedData(NewData, MergedInput.Tags, MergedInput.Pin));
		
			TArray<FPCGPoint>& NewPoints = NewData->GetMutablePoints();
		
			int32 MaxIterationPerTick = 10;
			
			// We are doing a Nest Loop O(n^2).
			// So whatever we say MaxIterationPerTick is will be squared.
			MaxIterationPerTick /= PointData->GetPoints().Num();
			MaxIterationPerTick = FMath::Max(1, MaxIterationPerTick);
		
			ProcessPointsIteration(Context, InputPoints, NewPoints, MaxIterationPerTick,[this, InputPoints, Diameter, MaxNeighbors, Context, b2D](int32 Index, FPCGPoint& OutPoint)->bool
			{
				int32 NeighborCount = 0;
				
				for (FPCGPoint Point : InputPoints)
				{
					FVector CurrentLocation = OutPoint.Transform.GetLocation();
					FVector OtherLocation = Point.Transform.GetLocation();
		
					if(b2D)
					{
						CurrentLocation.Z = 0;
						OtherLocation.Z = 0;
					}
					
					if((CurrentLocation - OtherLocation).Length() <= Diameter)
					{
						NeighborCount++;
					}
				}
				
				OutPoint.Density = (float)NeighborCount / (float)MaxNeighbors;
				
				return true;
			}, FinishExecuteFunction(Context));*/

			TArray<FPCGTaggedData>& Outputs = Context->OutputData.TaggedData;
			
			PCG::ProcessPointsSynchronous(Context, Inputs, Outputs,[this, Diameter, MaxNeighbors, Context, &Inputs, b2D](FPCGPoint& OutPoint)->bool
			{
				int32 NeighborCount = 0;
			
				for (FPCGTaggedData Input : Inputs)
				{
					const UPCGPointData* PointData = GetPointData(Context, Input);
	
					if(!PointData)
					{
						PCGE_LOG(Warning, GraphAndLog, FText::FromString("Input Data is not a Point or Spatial Data"));
						continue;
					}
	
					for (FPCGPoint Point : PointData->GetPoints())
					{
						FVector CurrentLocation = OutPoint.Transform.GetLocation();
						FVector OtherLocation = Point.Transform.GetLocation();
	
						if(b2D)
						{
							CurrentLocation.Z = 0;
							OtherLocation.Z = 0;
						}
					
						if((CurrentLocation - OtherLocation).Length() <= Diameter)
						{
							NeighborCount++;
						}
					}
				}
	
				OutPoint.Density = (float)NeighborCount / (float)MaxNeighbors;
			
				return true;
			});
		}

		if(bLogTime) UE_LOG(LogPCG, Warning, TEXT("Density From Neighbors took %f ms to execute"), (FPlatformTime::Seconds() - RawStartTime) * 1000);
		
		return true;
	},
	// On Completion, Finish the Async Node
	[Context]
	{
		FinishExecute(Context);
	});
	
	return true;
}

FText UPCGDensityFromNeighborsSettings::GetNodeTooltipText() const
{
	return NSLOCTEXT("PCGDensityFromNeighborsSettings", "NodeTooltip", "Calculates the Density of each point based on the normalized number of neighbors within a radius. \n This is perfectly accurate version of the 'DistanceToNeighbors' node but is also much more expensive operation. To avoid performance issues, this node is fully multithreaded and will should not affect performance of the game thread.");
}
