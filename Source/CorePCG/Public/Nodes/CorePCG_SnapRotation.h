// © 2021, Brock Marsh. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/CorePCGMacros.h"
#include "CorePCG_SnapRotation.generated.h"

class FPCGSnapRotationElement : public FPCGPointProcessingElementBase
{
protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
};


UCLASS(BlueprintType, ClassGroup = (Procedural))
class UPCGSnapRotationSettings : public UPCGSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings", meta=(PCG_Overridable))
	float SnapSize = 90.f;
	
#if	WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return FName(TEXT("SnapRotation")); }

	virtual FText GetDefaultNodeTitle() const override
	{
		return NSLOCTEXT("PCGSnapRotationSettings", "NodeTitle", "Snap Rotation");
	}

	virtual FText GetNodeTooltipText() const override { return NSLOCTEXT("PCGSnapRotationSettings", "NodeTooltip", "Snaps the points rotation to the desired snap size. Example: Rotator(0,10,60) with a Snap Size of 50 = Rotator(0,0,50)"); }

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

	virtual FPCGElementPtr CreateElement() const override
	{
		return MakeShared<FPCGSnapRotationElement>();
	};
};