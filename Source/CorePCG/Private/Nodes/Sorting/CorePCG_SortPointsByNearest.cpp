// © 2021, Brock Marsh. All rights reserved.

#include "Nodes/Sorting/CorePCG_SortPointsByNearest.h"

#include "PCGComponent.h"
#include "PCGContext.h"
#include "Data/PCGPointData.h"
#include "Data/PCGSpatialData.h"
#include "Helpers/CorePCGLibrary.h"

int32 GetFurthestPoint(TArray<FPCGPoint>& InPoints)
{
	if(InPoints.IsEmpty()) return -1;

	double MaxDistance = -MAX_FLT;
	int32 MaxIndex = -1;

	for(int32 i = 0; i < InPoints.Num(); i++)
	{
		// Add X, Y, and Z to get the distance from the origin. This will always give us the Top Right Corner
		const double Distance = InPoints[i].Transform.GetLocation().X + InPoints[i].Transform.GetLocation().Y + InPoints[i].Transform.GetLocation().Z;

		if(Distance > MaxDistance)
		{
			MaxDistance = Distance;
			MaxIndex = i;
		}
	}

	return MaxIndex;
}

void Recursive_AddNearestPoint(const FPCGPoint& Point, TArray<FPCGPoint>& InPoints, TArray<FPCGPoint>& OutPoints)
{
	if(InPoints.IsEmpty()) return;
	
	double MinDistance = MAX_FLT;
	int32 MinIndex = -1;

	for(int32 i = 0; i < InPoints.Num(); i++)
	{
		const double Distance = FVector::DistSquared(Point.Transform.GetLocation(), InPoints[i].Transform.GetLocation());

		if(Distance < MinDistance)
		{
			MinDistance = Distance;
			MinIndex = i;
		}
	}

	const FPCGPoint ClosestPoint = InPoints[MinIndex];

	InPoints.RemoveAtSwap(MinIndex);
	OutPoints.Add(ClosestPoint);

	Recursive_AddNearestPoint(ClosestPoint, InPoints, OutPoints);
}

bool FPCGSortPointsByNearestElement::ExecuteInternal(FPCGContext* Context) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FPCGSortPointsByNearestElement::Execute);
	
	const UPCGSortPointsByNearestSettings* Settings = Context->GetInputSettings<UPCGSortPointsByNearestSettings>();

	check(Settings);

	const TArray<FPCGTaggedData> Inputs = Context->InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel);
	TArray<FPCGTaggedData>& Outputs = Context->OutputData.TaggedData;

	if(Inputs.IsEmpty()) return true;
	
	const UPCGSpatialData* SpatialData = UCorePCGLibrary::GetMergeSpatialData(*Context).Data;

	const UPCGPointData* PointData = SpatialData->ToPointData(Context);

	if(PointData->GetPoints().IsEmpty()) return true;

	TArray<FPCGPoint> SortedPoints;
	SortedPoints.Reserve(PointData->GetPoints().Num());

	TArray<FPCGPoint> CopyPoints = PointData->GetPoints();

	// Set First point from the Corner of the Point Data Bounds
	int32 Index = GetFurthestPoint(CopyPoints);

	const FPCGPoint FirstPoint = CopyPoints[Index];
	CopyPoints.RemoveAtSwap(Index);
	
	SortedPoints.Add(FirstPoint);

	UE_LOG(LogTemp, Error, TEXT("Furtherst Point Index: %d"), Index);

	Recursive_AddNearestPoint(FirstPoint, CopyPoints, SortedPoints);

	UPCGPointData* NewPointData = NewObject<UPCGPointData>();
	NewPointData->InitializeFromData(PointData);
	NewPointData->SetPoints(SortedPoints);
	
	if(Outputs.IsValidIndex(0)) Outputs[0].Data = NewPointData;
	else Outputs.Emplace(NewPointData);

	return true;
}