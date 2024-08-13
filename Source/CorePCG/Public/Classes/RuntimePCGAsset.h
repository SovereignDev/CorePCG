// © 2021, Brock Marsh. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "PCGGraph.h"
#include "RuntimePCGAsset.generated.h"

/**
 *	A Wrapper Asset for placing PCG Graphs as a Runtime PCG Actor.
 */
UCLASS(BlueprintType)
class COREPCG_API URuntimePCGAsset : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PCG")
	TObjectPtr<UPCGGraph> Graph;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PCG")
	FVector VolumeScale = FVector(25.f, 25.f, 10.f);
};