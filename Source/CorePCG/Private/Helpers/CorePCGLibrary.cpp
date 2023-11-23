// © 2021, Brock Marsh. All rights reserved.


#include "Helpers/CorePCGLibrary.h"

#include "PCGContext.h"
#include "Data/PCGPointData.h"

FPCGDataArrayWrapper UCorePCGLibrary::GetInputData(const FPCGDataCollection& Input)
{
	return GetInputData<UPCGData>(Input);
}

FPCGDataArrayWrapper UCorePCGLibrary::GetInputDataByPin(const FPCGDataCollection& Input, const FName Pin)
{
	return GetInputDataByPin<UPCGData>(Input, Pin);
}

FPCGSpatialDataArrayWrapper UCorePCGLibrary::GetInputSpatialData(const FPCGDataCollection& Input)
{
	return GetInputData<UPCGSpatialData>(Input);
}

FPCGSpatialDataArrayWrapper UCorePCGLibrary::GetAllInputSpatialData(const FPCGDataCollection& Input)
{
	return GetAllInputData<UPCGSpatialData>(Input);
}

FPCGSpatialDataArrayWrapper UCorePCGLibrary::GetInputSpatialDataByPin(const FPCGDataCollection& Input, const FName Pin)
{
	return GetInputDataByPin<UPCGSpatialData>(Input, Pin);
}

FPCGPointDataArrayWrapper UCorePCGLibrary::GetInputPointData(const FPCGDataCollection& Input)
{
	return GetInputData<UPCGPointData>(Input);
}

FPCGPointDataArrayWrapper UCorePCGLibrary::GetAllInputPointData(const FPCGDataCollection& Input)
{
	return GetAllInputData<UPCGPointData>(Input);
}

FPCGPointDataArrayWrapper UCorePCGLibrary::GetInputPointDataByPin(const FPCGDataCollection& Input, const FName Pin)
{
	return GetInputDataByPin<UPCGPointData>(Input, Pin);
}

TArray<FPCGPoint> UCorePCGLibrary::GetPointsFromPointDataArray(TArray<UPCGPointData*> Array)
{
	TArray<FPCGPoint> Points;
	
	for (const UPCGPointData* Data : Array)
	{
		if(!Data) continue;

		Points.Append(Data->GetPoints());
	}

	return Points;
}

FPCGUnionSpatialDataWrapper UCorePCGLibrary::GetInputUnionSpatialData(const FPCGDataCollection& Input)
{
	const FPCGUnionSpatialDataWrapper OutUnion = GetInputUnionSpatialDataByPin(Input, "In");
	if(OutUnion.Data) return OutUnion;

	TArray<FPCGTaggedData> TaggedInputs = Input.GetInputs();
	if(TaggedInputs.IsEmpty()) return OutUnion;

	return GetInputUnionSpatialDataByPin(Input, TaggedInputs[0].Pin);
}

FPCGUnionSpatialDataWrapper UCorePCGLibrary::GetInputUnionSpatialDataByPin(const FPCGDataCollection& Input, const FName Pin)
{
	bool bOutUnion;
	return Input.GetSpatialUnionOfInputsByPin(Pin, bOutUnion);
}

FPCGUnionSpatialDataWrapper UCorePCGLibrary::GetMergeSpatialData(FPCGContext& Input)
{
	MergeInputData(&Input);
	
	if(Input.OutputData.GetInputs().IsEmpty() || !Input.OutputData.GetInputs()[0].Data)
	{
		UE_LOG(LogPCG, Error, TEXT("GetMergeSpatialData: No data in output of Core PCG Node %s | Task Name: %s"), *Input.Node->GetName(), *Input.GetTaskName());
		
		return FPCGUnionSpatialDataWrapper();
	}

	return Cast<UPCGSpatialData>(Input.OutputData.GetInputs()[0].Data.Get());
}

void UCorePCGLibrary::MergeInputData(FPCGContext* Context)
{
	if(!ensure(Context)) return;
	
	const bool bMergeMetadata = true;

	TArray<FPCGTaggedData> Sources = Context->InputData.GetInputs();
	TArray<FPCGTaggedData>& Outputs = Context->OutputData.TaggedData;

	if (Sources.IsEmpty()) return;

	UPCGPointData* TargetPointData = nullptr;
	FPCGTaggedData* TargetTaggedData = nullptr;

	// Prepare data & metadata
	// Done in two passes for futureproofing - expecting changes in the metadata attribute creation vs. usage in points
	for (const FPCGTaggedData& Source : Sources)
	{
		const UPCGPointData* SourcePointData = Cast<const UPCGPointData>(Source.Data);

		// If Source is not native point data, try to convert it from spatial data
		if (!SourcePointData)
		{
			const UPCGSpatialData* SourceSpatialData = Cast<const UPCGSpatialData>(Source.Data);
			
			if(SourceSpatialData)
			{
				SourcePointData = SourceSpatialData->ToPointData(Context);
			}
			else
			{
				UE_LOG(LogPCG, Error, TEXT("Unsupported data type in merge in Core PCG Node"))
				continue;
			}
		}
		
		if (!TargetPointData)
		{
			TargetPointData = NewObject<UPCGPointData>();
			TargetPointData->InitializeFromData(SourcePointData, nullptr, bMergeMetadata);

			TargetTaggedData = &(Outputs.Emplace_GetRef(Source));
			TargetTaggedData->Data = TargetPointData;
		}
		else
		{
			if (bMergeMetadata)
			{
				TargetPointData->Metadata->AddAttributes(SourcePointData->Metadata);
			}
			
			check(TargetTaggedData);
			TargetTaggedData->Tags.Append(Source.Tags); // TODO: only unique? if yes, fix union too
		}
	}

	// No valid input types
	if (!TargetPointData) return ;

	TArray<FPCGPoint>& TargetPoints = TargetPointData->GetMutablePoints();
	
	for(int32 SourceIndex = 0; SourceIndex < Sources.Num(); ++SourceIndex)
	{
		const UPCGPointData* SourcePointData = Cast<const UPCGPointData>(Sources[SourceIndex].Data);

		if (!SourcePointData)
		{
			continue;
		}

		int32 PointOffset = TargetPoints.Num();
		TargetPoints.Append(SourcePointData->GetPoints());

		if ((!bMergeMetadata || SourceIndex != 0) && !SourcePointData->GetPoints().IsEmpty())
		{
			TArrayView<FPCGPoint> TargetPointsSubset = MakeArrayView(&TargetPoints[PointOffset], SourcePointData->GetPoints().Num());
			for (FPCGPoint& Point : TargetPointsSubset)
			{
				Point.MetadataEntry = PCGInvalidEntryKey;
			}

			if (bMergeMetadata && TargetPointData->Metadata && SourcePointData->Metadata && SourcePointData->Metadata->GetAttributeCount() > 0)
			{
				TargetPointData->Metadata->SetPointAttributes(MakeArrayView(SourcePointData->GetPoints()), SourcePointData->Metadata, TargetPointsSubset);
			}
		}
	}
}
