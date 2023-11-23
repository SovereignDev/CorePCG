// © 2021, Brock Marsh. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "PCGPin.h"
#include "PCGSettings.h"
#include "Elements/PCGPointProcessingElementBase.h"
#include "Helpers/CorePCGAsyncHelpers.h"

#include "CorePCG_AssignMesh.generated.h"

USTRUCT(BlueprintType)
struct FWeightedMeshToPoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weighted Mesh")
	TSoftObjectPtr<UStaticMesh> Mesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weighted Mesh")
	float Weight = 1.f;

	/* Preloaded and cached bounds to avoid needing to load the mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weighted Mesh")
	FBox Bounds = FBox(EForceInit::ForceInitToZero);
	
	FORCEINLINE bool operator==(const FWeightedMeshToPoint& Other) const
	{
		return Mesh == Other.Mesh;
	}
	FORCEINLINE bool operator==(const TSoftObjectPtr<UStaticMesh>& Other) const
	{
		return Mesh == Other;
	}

	FORCEINLINE operator bool() const
	{
		return !Mesh.IsNull();
	}
};

/**
 *  Assigns a Mesh to the Points and updates the Extents with the Given Mesh's bounds
 */
UCLASS(BlueprintType, ClassGroup = (Procedural))
class UPCGAssignMeshSettings : public UPCGSettings
{
	GENERATED_BODY()
public:
	UPCGAssignMeshSettings();
	
	//~Begin UPCGSettings interface
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return FName(TEXT("AssignMesh")); }
	virtual FText GetDefaultNodeTitle() const override { return NSLOCTEXT("PCGAssignMeshSettings", "NodeTitle", "Assign Mesh"); }
	virtual FLinearColor GetNodeTitleColor() const override { return FLinearColor(1.f,0.6f, 0.f); };
	virtual FText GetNodeTooltipText() const override { return NSLOCTEXT("PCGAssignMeshSettings", "NodeTooltip", "Assigns a Mesh to the Points and updates the Extents with the Given Mesh's bounds"); }
	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::Spatial; }
#endif

protected:
	virtual TArray<FPCGPinProperties> InputPinProperties() const override { return Super::DefaultPointInputPinProperties(); }
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override { return Super::DefaultPointOutputPinProperties(); }
	virtual FPCGElementPtr CreateElement() const override;
	//~End UPCGSettings interface

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	TArray<FWeightedMeshToPoint> Meshes;

	/* If true, the Points bounds will be set from the selected Meshes Bounds */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	bool bSetBounds = true;

	/* Scale to apply to the Bounds when using Mesh bounds */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable, EditCondition="bSetBounds", EditConditionHides))
	float BoundsScale = 1.f;

#if WITH_EDITOR
	/* Adds the Meshes that are currently Selected in the Content Browser to the Mesh Array */
	UFUNCTION(CallInEditor, Category = "Settings")
	void AddSelectedMeshes();

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};

class FPCGAssignMeshElement : public FCorePCGPointProcessingAsyncElementBase
{
protected:
	virtual bool AsyncExecuteInternal(FCorePCGAsyncContext* Context) const override;
};