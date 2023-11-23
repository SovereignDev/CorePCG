// © 2021, Brock Marsh. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Helpers/CorePCGAsyncHelpers.h"
#include "Misc/CorePCGMacros.h"
#include "CorePCG_QuantizeFilter.generated.h"

/**
 * 
 */

class FPCGQuantizeFilterElement : public FCorePCGAsyncElementBase
{
protected:
	virtual bool AsyncExecuteInternal(FCorePCGAsyncContext* Context) const override;
};

UCLASS(BlueprintType, ClassGroup = (Procedural))
class UPCGQuantizeFilterSettings : public UPCGSettings
{
	GENERATED_BODY()

public:
	UPCGQuantizeFilterSettings() { bUseSeed = true; }
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings", meta=(PCG_Overridable))
	float CellSize = 1000.f;

	/** Points that are within CellSize*OverlapThreshold will be removed */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings", meta=(PCG_Overridable))
	float OverlapThreshold = 0.5f;
	
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return FName(TEXT("Quantize Filter")); }

	virtual FText GetDefaultNodeTitle() const override
	{
		return NSLOCTEXT("PCGQuantizeFilterSettings", "NodeTitle", "Quantize Filter");
	}

	// 0.217, 0.0823, 0.6875

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

	virtual FPCGElementPtr CreateElement() const override { return MakeShared<FPCGQuantizeFilterElement>(); }
};
