// © 2021, Brock Marsh. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/CorePCGMacros.h"
#include "CorePCG_SortPointsCounterClockwise.generated.h"

/**
 * 
 */
class FPCGSortPointsCounterClockwiseElement : public FPCGPointProcessingElementBase
{
protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
};

UCLASS(BlueprintType, ClassGroup = (Procedural))
class UPCGSortPointsCounterClockwiseSettings : public UPCGSettings
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return FName(TEXT("Sort Points Counter Clockwise")); }

	virtual FText GetDefaultNodeTitle() const override
	{
		return NSLOCTEXT("PCGSortPointsCounterClockwiseSettings", "NodeTitle", "Sort Points Counter Clockwise");
	}

	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::Filter; }
#endif

protected:
	virtual TArray<FPCGPinProperties> InputPinProperties() const override
	{
		return Super::DefaultPointInputPinProperties();
	}

	virtual TArray<FPCGPinProperties> OutputPinProperties() const override
	{
		return Super::DefaultPointOutputPinProperties();
	}

	virtual FPCGElementPtr CreateElement() const override
	{
		return MakeShared<FPCGSortPointsCounterClockwiseElement>();
	}
};
