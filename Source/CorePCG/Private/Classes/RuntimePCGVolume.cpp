// © 2021, Brock Marsh. All rights reserved.


#include "Classes/RuntimePCGVolume.h"

#include "PCGComponent.h"
#include "Classes/RuntimePCGAsset.h"
#include "Net/UnrealNetwork.h"

ARuntimePCGVolume::ARuntimePCGVolume(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if(PCGComponent)
	{
		PCGComponent->GenerationTrigger = EPCGComponentGenerationTrigger::GenerateOnDemand;
	}

	bReplicates = true;
	PCGComponent->bRegenerateInEditor = false;
}

void ARuntimePCGVolume::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARuntimePCGVolume, ReplicatedSeed);
}

void ARuntimePCGVolume::SetAsset(URuntimePCGAsset* NewAsset)
{
	Asset = NewAsset;
	PCGComponent->SetGraphLocal(Asset->Graph);
}

void ARuntimePCGVolume::BeginPlay()
{
	Super::BeginPlay();

	if(bGenerateAtBeginPlay)
	{
		GenerateNewSeed();
	}
}

void ARuntimePCGVolume::OnRep_Seed()
{
	OnNewSeedGenerated();
}

void ARuntimePCGVolume::GenerateNewSeed(bool bForce)
{
	ReplicatedSeed = FMath::Rand();

	OnNewSeedGenerated();
}

void ARuntimePCGVolume::OnNewSeedGenerated(bool bForce)
{
	if(!PCGComponent) return;

	PCGComponent->Seed = ReplicatedSeed;
	PCGComponent->GenerateLocal(true);
}

#if WITH_EDITOR
void ARuntimePCGVolume::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if(PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ARuntimePCGVolume, Asset))
	{
		PCGComponent->SetGraphLocal(Asset->Graph);
	}
}
#endif