// © 2021, Brock Marsh. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "PCGVolume.h"
#include "RuntimePCGVolume.generated.h"

/**
 *	An actor that generates and replicates a new seed at begin play before executing the PCG Graph.
 */
UCLASS(Blueprintable)
class COREPCG_API ARuntimePCGVolume : public APCGVolume
{
	GENERATED_BODY()

public:
	ARuntimePCGVolume(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PCG")
	TObjectPtr<class URuntimePCGAsset> Asset;
	
	UFUNCTION(BlueprintCallable, Category = "PCG")
	void SetAsset(class URuntimePCGAsset* NewAsset);
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "PCG")
	bool bGenerateAtBeginPlay = true;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "PCG", ReplicatedUsing="OnRep_Seed")
	int32 ReplicatedSeed = 69;

	UFUNCTION()
	void OnRep_Seed();

public:
	UFUNCTION(BlueprintCallable, Category = "PCG")
	void GenerateNewSeed(bool bForce = false);

protected:
	void OnNewSeedGenerated(bool bForce = false);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};