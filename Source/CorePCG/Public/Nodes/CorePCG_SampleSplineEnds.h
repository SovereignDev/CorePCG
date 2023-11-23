// © 2021, Brock Marsh. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/CorePCGMacros.h"
#include "CorePCG_SampleSplineEnds.generated.h"

/**
 * 
 */

class FPCGSampleSplineEndsElement : public FPCGPointProcessingElementBase
{
protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
};

UCLASS(BlueprintType, ClassGroup = (Procedural))
class UPCGSampleSplineEndsSettings : public UPCGSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings", meta=(PCG_Overridable))
	bool bSampleSplineStart = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Settings", meta=(PCG_Overridable))
	bool bSampleSplineEnd = true;
	
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return FName(TEXT("Sample Spline Ends")); }

	virtual FText GetDefaultNodeTitle() const override
	{
		return NSLOCTEXT("PCGSampleSplineEndsSettings", "NodeTitle", "Sample Spline Ends");
	}

	virtual FText GetNodeTooltipText() const override;

	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::Sampler; }
#endif

protected:
	virtual TArray<FPCGPinProperties> InputPinProperties() const override
	{
		FPCGPinProperties InputPin;
		InputPin.Label = "Spline";
		InputPin.AllowedTypes = EPCGDataType::Spline;
		InputPin.bAllowMultipleConnections = false;
		InputPin.bAllowMultipleData = false;

		return {InputPin};
	}

	virtual TArray<FPCGPinProperties> OutputPinProperties() const override
	{
		return Super::DefaultPointOutputPinProperties();
	}

	virtual FPCGElementPtr CreateElement() const override { return MakeShared<FPCGSampleSplineEndsElement>(); }
};
