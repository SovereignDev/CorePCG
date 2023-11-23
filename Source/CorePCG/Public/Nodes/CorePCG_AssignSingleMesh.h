// © 2021, Brock Marsh. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "PCGSettings.h"
#include "Helpers/CorePCGAsyncHelpers.h"
#include "UObject/Object.h"
#include "CorePCG_AssignSingleMesh.generated.h"

class FPCGAssignSingleMeshElement : public FCorePCGPointProcessingAsyncElementBase
{
protected:
	virtual bool AsyncExecuteInternal(FCorePCGAsyncContext* Context) const override;
};

/**
 *	Async Loads a Static Mesh Soft Ptr and assigns it to every point.
 */
UCLASS(BlueprintType, ClassGroup = (Procedural))
class UPCGAssignSingleMeshSettings : public UPCGSettings
{
	GENERATED_BODY()
public:
	UPCGAssignSingleMeshSettings();
	
	//~Begin UPCGSettings interface
#if WITH_EDITOR
	virtual FName GetDefaultNodeName() const override { return FName(TEXT("AssignSingleMesh")); }
	virtual FText GetDefaultNodeTitle() const override { return NSLOCTEXT("PCGAssignSingleMeshSettings", "NodeTitle", "Assign Single Mesh"); }
	virtual FLinearColor GetNodeTitleColor() const override { return FLinearColor(1.f,0.6f, 0.f); };
	virtual FText GetNodeTooltipText() const override { return NSLOCTEXT("PCGAssignSingleMeshSettings", "NodeTooltip", "Async Loads a Static Mesh Soft Ptr and assigns it to every point"); }
	virtual EPCGSettingsType GetType() const override { return EPCGSettingsType::Spatial; }
#endif

protected:
	virtual TArray<FPCGPinProperties> InputPinProperties() const override { return Super::DefaultPointInputPinProperties(); }
	virtual TArray<FPCGPinProperties> OutputPinProperties() const override { return Super::DefaultPointOutputPinProperties(); }
	virtual FPCGElementPtr CreateElement() const override { return MakeShared<FPCGAssignSingleMeshElement>(); }
	//~End UPCGSettings interface

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	TSoftObjectPtr<UStaticMesh> Mesh;
	
	/* If true, the Points bounds will be set from the selected Meshes Bounds */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable))
	bool bSetBounds = true;

	/* Scale to apply to the Bounds when using Mesh bounds */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings", meta = (PCG_Overridable, EditCondition="bSetBounds", EditConditionHides))
	float BoundsScale = 1.f;
};