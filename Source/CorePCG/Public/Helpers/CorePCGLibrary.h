// © 2021, Brock Marsh. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "PCGData.h"
#include "PCGPoint.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CorePCGLibrary.generated.h"

/**
 * 
 */

class UPCGPointData;

USTRUCT(BlueprintType)
struct FPCGDataArrayWrapper
{
	GENERATED_BODY()

	FPCGDataArrayWrapper(){}
	FPCGDataArrayWrapper(const TArray<TObjectPtr<const UPCGData>>& InData) : Data(InData) {}

	UPROPERTY(BlueprintReadOnly, Category="PCG")
	TArray<TObjectPtr<const UPCGData>> Data;
};

USTRUCT(BlueprintType)
struct FPCGSpatialDataArrayWrapper
{
	GENERATED_BODY()

	FPCGSpatialDataArrayWrapper(){}
	FPCGSpatialDataArrayWrapper(const TArray<TObjectPtr<const UPCGSpatialData>>& InData) : Data(InData) {}

	UPROPERTY(BlueprintReadOnly, Category="PCG")
	TArray<TObjectPtr<const UPCGSpatialData>> Data;
};

USTRUCT(BlueprintType)
struct FPCGPointDataArrayWrapper
{
	GENERATED_BODY()

	FPCGPointDataArrayWrapper(){}
	FPCGPointDataArrayWrapper(const TArray<TObjectPtr<const UPCGPointData>>& InData) : Data(InData) {}

	UPROPERTY(BlueprintReadOnly, Category="PCG")
	TArray<TObjectPtr<const UPCGPointData>> Data;
};

USTRUCT(BlueprintType)
struct FPCGUnionSpatialDataWrapper
{
	GENERATED_BODY()

	FPCGUnionSpatialDataWrapper(){}
	FPCGUnionSpatialDataWrapper(const UPCGSpatialData* In) : Data(In) {}

	UPROPERTY(BlueprintReadOnly, Category="PCG")
	TObjectPtr<const UPCGSpatialData> Data;
};

UCLASS()
class COREPCG_API UCorePCGLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	template<class T = UPCGData>
	static TArray<TObjectPtr<const T>> GetInputData(const FPCGDataCollection& Input)
	{
		TArray<TObjectPtr<const T>> OutData = GetInputDataByPin<T>(Input, "In");

		if(!OutData.IsEmpty()) return OutData;
		
		const TArray<FPCGTaggedData> TaggedData = Input.GetInputs();
		if(TaggedData.IsEmpty()) return OutData;
		
		return GetInputDataByPin<T>(Input, TaggedData[0].Pin);
	}
	
	template<class T = UPCGData>
	static TArray<TObjectPtr<const T>> GetInputDataByPin(const FPCGDataCollection& Input, const FName Pin)
	{
		const TArray<FPCGTaggedData> TaggedInputs = Input.GetInputsByPin(Pin);
		
		TArray<TObjectPtr<const T>> OutData;
		OutData.Reserve(TaggedInputs.Num());

		for (const FPCGTaggedData& TaggedInput : TaggedInputs) OutData.Add(Cast<T>(TaggedInput.Data));
		
		return OutData;
	}

	template<class T = UPCGData>
	static TArray<TObjectPtr<const T>> GetAllInputData(const FPCGDataCollection& Input)
	{
		const TArray<FPCGTaggedData> TaggedData = Input.GetInputs();

		TArray<TObjectPtr<const T>> OutData;
		OutData.Reserve(TaggedData.Num());

		for (const FPCGTaggedData& Data : TaggedData) OutData.Add(Cast<T>(Data.Data));

		return OutData;
	}
	
	UFUNCTION(BlueprintCallable, Category="PCG|Core")
	static FPCGDataArrayWrapper GetInputData(const FPCGDataCollection& Input);
	UFUNCTION(BlueprintCallable, Category="PCG|Core")
	static FPCGDataArrayWrapper GetInputDataByPin(const FPCGDataCollection& Input, const FName Pin);

	UFUNCTION(BlueprintCallable, Category="PCG|Core")
	static FPCGSpatialDataArrayWrapper GetInputSpatialData(const FPCGDataCollection& Input);
	UFUNCTION(BlueprintCallable, Category="PCG|Core")
	static FPCGSpatialDataArrayWrapper GetAllInputSpatialData(const FPCGDataCollection& Input);
	UFUNCTION(BlueprintCallable, Category="PCG|Core")
	static FPCGSpatialDataArrayWrapper GetInputSpatialDataByPin(const FPCGDataCollection& Input, const FName Pin);

	UFUNCTION(BlueprintCallable, Category="PCG|Core")
	static FPCGPointDataArrayWrapper GetInputPointData(const FPCGDataCollection& Input);
	UFUNCTION(BlueprintCallable, Category="PCG|Core")
	static FPCGPointDataArrayWrapper GetAllInputPointData(const FPCGDataCollection& Input);
	UFUNCTION(BlueprintCallable, Category="PCG|Core")
	static FPCGPointDataArrayWrapper GetInputPointDataByPin(const FPCGDataCollection& Input, const FName Pin);

	UFUNCTION(BlueprintCallable, Category="PCG|Core")
	static TArray<FPCGPoint> GetPointsFromPointDataArray(TArray<UPCGPointData*> Array);
	
	UFUNCTION(BlueprintCallable, Category="PCG|Core")
	static FPCGUnionSpatialDataWrapper GetInputUnionSpatialData(const FPCGDataCollection& Input);
	UFUNCTION(BlueprintCallable, Category="PCG|Core")
	static FPCGUnionSpatialDataWrapper GetInputUnionSpatialDataByPin(const FPCGDataCollection& Input, const FName Pin);

	UFUNCTION(BlueprintCallable, Category="PCG|Core")
	static FPCGUnionSpatialDataWrapper GetMergeSpatialData(UPARAM(ref) struct FPCGContext& Input);

	static void MergeInputData(struct FPCGContext* Context);
};