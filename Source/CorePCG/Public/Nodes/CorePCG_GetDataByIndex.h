// © 2021, Brock Marsh. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/CorePCGMacros.h"
#include "UObject/Object.h"
#include "CorePCG_GetDataByIndex.generated.h"

/**
 * 
 */
class FPCGGetDataByIndexElement : public FPCGPointProcessingElementBase
{
protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
};

UCLASS(BlueprintType, ClassGroup = (Procedural))
class UPCGGetDataByIndexSettings : public UPCGSettings
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG", meta=(PCG_Overridable))
	int32 Index = 0;

	// TODO Expose center location as an Input
	
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return FName(TEXT("GetDataByIndex")); }

	virtual FText GetDefaultNodeTitle() const override
	{
		return NSLOCTEXT("PCGGetDataByIndexSettings", "NodeTitle", "GetDataByIndex");
	}
	virtual FText GetNodeTooltipText() const override { return NSLOCTEXT("PCGGetDataByIndexSettings", "NodeTooltip", "Gets a Spatial Data from the input array by index."); }

	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::Spatial; }
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

	virtual FPCGElementPtr CreateElement() const override { return MakeShared<FPCGGetDataByIndexElement>(); }
};