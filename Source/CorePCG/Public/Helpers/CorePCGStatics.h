// © 2021, Brock Marsh. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "PCGContext.h"
#include "PCGData.h"
#include "PCGParamData.h"
#include "Data/PCGPointData.h"

/**
 * 
 */
namespace PCG
{
	inline bool HasAnyInputs(const FPCGContext* Context, const FName PinName = PCGPinConstants::DefaultInputLabel)
	{
		return Context->InputData.GetInputsByPin(PinName).IsValidIndex(0);
	}
	inline TArray<FPCGTaggedData> GetInputs(const FPCGContext* Context, const FName PinName = PCGPinConstants::DefaultInputLabel)
	{
		return Context->InputData.GetInputsByPin(PinName);
	}
	inline TArray<FPCGTaggedData>& GetOutputs(FPCGContext* Context)
	{
		return Context->OutputData.TaggedData;
	}
	
	/* Gets the Meta Data from a PCG Data by checking all child classes that declare a meta data object */
	class UPCGMetadata* GetMetaData(UPCGData* Data);

	/** Takes in an Array of Tagged Data and Sets all meta data variables to the specified value */
	inline void SetMetaData(TArray<FPCGTaggedData>& InData, class UPCGMetadata* MetaData)
	{
		for (FPCGTaggedData& Data : InData)
		{
			UPCGSpatialData* SpatialData = Cast<UPCGSpatialData>(Data.Data);
			if(SpatialData)
			{
				SpatialData->Metadata = MetaData;
				continue;
			}

			UPCGParamData* ParamData = Cast<UPCGParamData>(Data.Data);
			if(ParamData)
			{
				ParamData->Metadata = MetaData;
				continue;
			}
		}
	}

	inline const UPCGPointData* AsPointData(FPCGContext* Context, const UPCGData* Data)
	{
		const UPCGPointData* PointData = Cast<UPCGPointData>(Data);
		if(!PointData)
		{
			const UPCGSpatialData* SData = Cast<UPCGSpatialData>(Data);
		
			if(!SData)
			{
				return nullptr;
			}

			PointData = SData->ToPointData(Context);
		}

		return PointData;
	}

	/**
	 *	Synchronously Process all the Input Points. This version is Constant. Any Changes to the Points will not be reflected in the Output.
	 */
	inline void ConstantProcessPointsSynchronous(FPCGContext* Context, const TArray<FPCGTaggedData>& Inputs, const TFunction<void(const FPCGPoint&)>& Lambda)
	{
		for (const FPCGTaggedData& Input : Inputs)
		{
			const UPCGPointData* PointData = AsPointData(Context, Input.Data);

			if (!PointData) continue;

			for (FPCGPoint CurrentPoint : PointData->GetPoints())
			{
				Lambda(CurrentPoint);
			}
		}
	}

	inline void ProcessPointsSynchronous(FPCGContext* Context, const TArray<FPCGTaggedData>& Inputs, TArray<FPCGTaggedData>& Output, const TFunction<bool(FPCGPoint&)>& Lambda)
	{
		Output.Reserve(Inputs.Num());
	
		for (const FPCGTaggedData& Input : Inputs)
		{
			const UPCGPointData* PointData = AsPointData(Context, Input.Data);
			if (!PointData) continue;
		
			UPCGPointData* NewData = NewObject<UPCGPointData>();
			NewData->InitializeFromData(PointData);
			Output.Add(FPCGTaggedData(NewData, Input.Tags, Input.Pin));

			TArray<FPCGPoint> NewPoints = PointData->GetPoints();
		
			for (auto It = NewPoints.CreateIterator(); It; ++It)
			{
				const bool bSuccess = Lambda(*It);

				if(!bSuccess) It.RemoveCurrent();
			}

			NewData->SetPoints(NewPoints);
		}
	}
}