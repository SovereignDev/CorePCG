// © 2021, Brock Marsh. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/CorePCGMacros.h"
#include "UObject/Object.h"
#include "CorePCG_AssignTag.generated.h"

/**
 * 
 */

class FPCGAssignTagElement : public FSimplePCGElement
{
protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
};

UCLASS(BlueprintType, ClassGroup = (Procedural))
class UPCGAssignTagSettings : public UPCGSettings
{
	GENERATED_BODY()

public:
	UPCGAssignTagSettings();
	
	/* The Tags to Assign to a Set of PCG Data*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings", meta=(PCG_Overridable))
	TArray<FString> Tags;
	
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return FName(TEXT("AssignTags")); }

	virtual FText GetDefaultNodeTitle() const override
	{
		return NSLOCTEXT("PCGAssignTagSettings", "NodeTitle", "Assign Tags");
	}

	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::Filter; }
#endif

protected:
	virtual TArray<FPCGPinProperties> InputPinProperties() const override
	{
		TArray<FPCGPinProperties> PinProperties;
		PinProperties.Emplace(PCGPinConstants::DefaultInputLabel, EPCGDataType::Any);

		return PinProperties;
	}

	virtual TArray<FPCGPinProperties> OutputPinProperties() const override
	{
		TArray<FPCGPinProperties> PinProperties;
		PinProperties.Emplace(PCGPinConstants::DefaultOutputLabel, EPCGDataType::Any);

		return PinProperties;
	}

	virtual FPCGElementPtr CreateElement() const override { return MakeShared<FPCGAssignTagElement>(); }
};
