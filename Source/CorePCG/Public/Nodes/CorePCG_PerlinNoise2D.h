// © 2021, Brock Marsh. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/CorePCGMacros.h"
#include "CorePCG_PerlinNoise2D.generated.h"


class FPCGPerlinNoise2DElement : public FPCGPointProcessingElementBase
{
protected:
	virtual bool ExecuteInternal(FPCGContext* Context) const override;
};

UCLASS(BlueprintType, ClassGroup = (Procedural))
class UPCGPerlinNoise2DSettings : public UPCGSettings
{
	GENERATED_BODY()

	UPCGPerlinNoise2DSettings();
	
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings", meta=(PCG_Overridable))
	float Scale = 1.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings", meta=(PCG_Overridable))
	float Frequency = 1.f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings", meta=(PCG_Overridable))
	float Power = 1.f;
	
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return FName(TEXT("PerlinNoise2D")); }

	virtual FText GetDefaultNodeTitle() const override
	{
		return NSLOCTEXT("PCGPerlinNoise2DSettings", "NodeTitle", "Perlin Noise 2D");
	}

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

	virtual FPCGElementPtr CreateElement() const override { return MakeShared<FPCGPerlinNoise2DElement>(); }
};