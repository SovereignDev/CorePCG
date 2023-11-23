// © 2021, Brock Marsh. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Elements/PCGSelfPruning.h"
#include "Misc/CorePCGMacros.h"
#include "UObject/Object.h"
#include "CorePCG_PreciseSelfPrune.generated.h"

/**
 * 
 */
namespace PCGSelfPruningElement
{
	COREPCG_API void ExecutePrecise(FPCGContext* Context, EPCGSelfPruningType PruningType);
}

class FPCGPreciseSelfPruneElement : public FPCGPointProcessingElementBase
{
protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
};

UCLASS(BlueprintType, ClassGroup = (Procedural), Category="TEST_WTF")
class UPCGPreciseSelfPruneSettings : public UPCGSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Settings, meta = (PCG_Overridable, InvalidEnumValues="None, RemoveDuplicates"))
	EPCGSelfPruningType PruningType;

#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return FName(TEXT("Precise Self Prune")); }

	virtual FText GetDefaultNodeTitle() const override
	{
		return NSLOCTEXT("PCGPreciseSelfPruneSettings", "Node Title", "Precise Self Prune");
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

	virtual FPCGElementPtr CreateElement() const override { return MakeShared<FPCGPreciseSelfPruneElement>(); }
};