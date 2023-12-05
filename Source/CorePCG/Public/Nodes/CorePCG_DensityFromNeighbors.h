// © 2021, Brock Marsh. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Helpers/CorePCGAsyncHelpers.h"
#include "Misc/CorePCGMacros.h"
#include "UObject/Object.h"
#include "CorePCG_DensityFromNeighbors.generated.h"

/**
 * 
 */
class FPCGDensityFromNeighborsElement : public FCorePCGPointProcessingAsyncElementBase
{
protected:
	//virtual bool ExecuteInternal(FPCGContext* Context) const override;
	virtual bool AsyncExecuteInternal(FCorePCGAsyncContext* Context) const override;
	
//	virtual bool AsyncExecuteInternal(FCorePCGAsyncContext* Context) const override;
};

UCLASS(BlueprintType, ClassGroup = (Procedural))
class UPCGDensityFromNeighborsSettings : public UPCGSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings", meta=(PCG_Overridable))
	float Radius = 150.f;

	// The Distance to Neighbors will be calculated in 2D space
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings", meta=(PCG_Overridable))
	bool bCalculate2DSpace = true;

	/* If all points have the Same Size then this node can be calculate faster */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings", meta=(PCG_Overridable))
	bool bHasConstantSize = false;

#if WITH_EDITORONLY_DATA
	/** Logs the execution time of each operation performed by this node */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Settings", meta=(PCG_Overridable))
	bool bLogTime = false;
#endif
	
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return FName(TEXT("Density From Neighbors")); }

	virtual FText GetDefaultNodeTitle() const override
	{
		return NSLOCTEXT("PCGDensityFromNeighborsSettings", "NodeTitle", "Density From Neighbors");
	}

	virtual FText GetNodeTooltipText() const override { return NSLOCTEXT("PCGDensityFromNeighborsSettings", "NodeTooltip", "Calculates the Density of each point based on the normalized number of neighbors within a radius. \n This is perfectly accurate version of the 'DistanceToNeighbors' node but is also much more expensive operation. To avoid performance issues, this node is fully multithreaded and will should not affect performance of the game thread."); }

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

	virtual FPCGElementPtr CreateElement() const override { return MakeShared<FPCGDensityFromNeighborsElement>(); }
};
