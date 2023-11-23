// © 2021, Brock Marsh. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "PCGSettings.h"
#include "Helpers/CorePCGAsyncHelpers.h"
#include "Misc/CorePCGMacros.h"
#include "CorePCG_AssignActor.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FWeightedActorToPoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weighted Mesh")
	TSoftClassPtr<AActor> Class = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weighted Mesh")
	float Weight = 1.f;

	/* Preloaded and cached bounds to avoid needing to load the mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weighted Mesh")
	FBox Bounds = FBox(EForceInit::ForceInitToZero);
	
	FORCEINLINE bool operator==(const FWeightedActorToPoint& Other) const
	{
		return Class == Other.Class;
	}
	FORCEINLINE bool operator==(const TSoftClassPtr<AActor>& Other) const
	{
		return Class == Other;
	}

	FORCEINLINE operator bool() const
	{
		return !Class.IsNull();
	}
};

class FPCGAssignActorElement : public FCorePCGPointProcessingAsyncElementBase
{
protected:
	virtual bool AsyncExecuteInternal(FCorePCGAsyncContext* Context) const override;
};

UCLASS(BlueprintType, ClassGroup = (Procedural))
class UPCGAssignActorSettings : public UPCGSettings
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return FName(TEXT("Assign Actor")); }
	virtual FText GetDefaultNodeTitle() const override { return NSLOCTEXT("PCGAssignActorSettings", "NodeTitle", "Assign Actor"); }
	virtual FLinearColor GetNodeTitleColor() const override { return FLinearColor(1.f,0.6f, 0.f); };
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

	virtual FPCGElementPtr CreateElement() const override { return MakeShared<FPCGAssignActorElement>(); }

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	TArray<FWeightedActorToPoint> Actors;

	/* If true, the Points bounds will be set from the selected Meshes Bounds */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	bool bSetBounds = true;

	/* Scale to apply to the Bounds when using Mesh bounds */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable, InlineEditConditionToggle="bSetBounds"))
	float BoundsScale = 1.f;

#if WITH_EDITORONLY_DATA
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	bool bCalculateBoundsFromCollision = true;
#endif

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
