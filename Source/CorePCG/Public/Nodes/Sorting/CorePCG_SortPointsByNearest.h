// © 2021, Brock Marsh. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/CorePCGMacros.h"
#include "UObject/Object.h"
#include "CorePCG_SortPointsByNearest.generated.h"

/**
 * 
 */
class FPCGSortPointsByNearestElement : public FSimplePCGElement
{
protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
};

UCLASS(BlueprintType, ClassGroup = (Procedural))
class UPCGSortPointsByNearestSettings : public UPCGSettings
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return FName(TEXT("Sort Points By Nearest")); }

	virtual FText GetDefaultNodeTitle() const override
	{
		return NSLOCTEXT("PCGSortPointsByNearestSettings", "NodeTitle", "Sort Points By Nearest");
	}

	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::Filter; }
#endif

protected:
	virtual TArray<FPCGPinProperties> InputPinProperties() const override
	{
		TArray<FPCGPinProperties> Properties;

		const FPCGPinProperties Pin(PCGPinConstants::DefaultInputLabel, EPCGDataType::Point);
		Properties.Add(Pin);
		
		return Properties;
	}

	virtual TArray<FPCGPinProperties> OutputPinProperties() const override
	{
		TArray<FPCGPinProperties> Properties;

		const FPCGPinProperties Pin(PCGPinConstants::DefaultInputLabel, EPCGDataType::Point);
		Properties.Add(Pin);
		
		return Properties;
	}

	virtual FPCGElementPtr CreateElement() const override { return MakeShared<FPCGSortPointsByNearestElement>(); }
};
