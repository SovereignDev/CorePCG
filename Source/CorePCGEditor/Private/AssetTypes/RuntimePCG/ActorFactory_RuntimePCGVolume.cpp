// © 2021, Brock Marsh. All rights reserved.


#include "AssetTypes/RuntimePCG/ActorFactory_RuntimePCGVolume.h"

#include "PCGComponent.h"
#include "PCGEngineSettings.h"
#include "PCGSubsystem.h"
#include "Classes/RuntimePCGAsset.h"
#include "Classes/RuntimePCGVolume.h"
#include "Elements/Framework/TypedElementRegistry.h"
#include "Elements/Interfaces/TypedElementObjectInterface.h"
#include "Subsystems/PlacementSubsystem.h"


#define LOCTEXT_NAMESPACE "ActorFactory_RuntimePCGVolume"

UActorFactory_RuntimePCGVolume::UActorFactory_RuntimePCGVolume(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DisplayName = LOCTEXT("RuntimePCGVolumeDisplayName", "Runtime PCG Volume");
	NewActorClass = ARuntimePCGVolume::StaticClass();
	bUseSurfaceOrientation = false;
}

bool UActorFactory_RuntimePCGVolume::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	if (!AssetData.IsValid() || !AssetData.IsInstanceOf(URuntimePCGAsset::StaticClass()))
	{
		OutErrorMsg = NSLOCTEXT("CanCreateActor", "NotRuntimePCGAsset", "A valid Runtime PCG Asset must be specified.");
		return false;
	}

	return true;
}

UClass* UActorFactory_RuntimePCGVolume::GetDefaultActorClass(const FAssetData& AssetData)
{
	return ARuntimePCGVolume::StaticClass();
}

void UActorFactory_RuntimePCGVolume::PostPlaceAsset(TArrayView<const FTypedElementHandle> InElementHandles, const FAssetPlacementInfo& InPlacementInfo, const FPlacementOptions& InPlacementOptions)
{
	Super::PostPlaceAsset(InElementHandles, InPlacementInfo, InPlacementOptions);
	
	// Preview elements are created while dragging an asset, but before dropping.
	if (InPlacementOptions.bIsCreatingPreviewElements) return;

	for (const FTypedElementHandle& PlacedElement : InElementHandles)
	{
		TTypedElement<ITypedElementObjectInterface> ObjectInterface = UTypedElementRegistry::GetInstance()->GetElement<ITypedElementObjectInterface>(PlacedElement);
		AActor* NewActor = ObjectInterface ? ObjectInterface.GetObjectAs<AActor>() : nullptr;
		if (!NewActor) continue;

		URuntimePCGAsset* NewAsset = Cast<URuntimePCGAsset>(InPlacementInfo.AssetToPlace.GetAsset());
		const UPCGEngineSettings* Settings = GetDefault<UPCGEngineSettings>();
		if (!NewAsset || !Settings)
		{
			continue;
		}

		ARuntimePCGVolume* PCGVolume = CastChecked<ARuntimePCGVolume>(NewActor);
		PCGVolume->SetActorScale3D(NewAsset->VolumeScale);
		PCGVolume->SetAsset(NewAsset);

		UPCGComponent* PCGComponent = PCGVolume->PCGComponent;
		PCGComponent->Seed = FMath::Rand();

		if (Settings->bGenerateOnDrop)
		{
			if (UPCGSubsystem* Subsystem = PCGComponent->GetSubsystem())
			{
				TWeakObjectPtr<UPCGComponent> ComponentPtr(PCGComponent);

				// Schedule a task to generate this component.
				// We cannot generate the component right away because after PostSpawnActor is called, all
				// actor components are unregistered and re-registered, which cancels component generation
				Subsystem->ScheduleGeneric([ComponentPtr]()
				{
					if (UPCGComponent* Component = ComponentPtr.Get())
					{
						// If the component is not valid anymore, just early out.
						if (!IsValid(Component))
						{
							return true;
						}

						Component->Generate();
					}

					return true;
				}, PCGComponent, /*TaskDependencies=*/{});
			}
		}
	}
}

UObject* UActorFactory_RuntimePCGVolume::GetAssetFromActorInstance(AActor* ActorInstance)
{
	return Cast<ARuntimePCGVolume>(ActorInstance)->Asset;
}

bool UActorFactory_RuntimePCGVolume::PreSpawnActor(UObject* Asset, FTransform& InOutLocation)
{
	InOutLocation.SetLocation(InOutLocation.GetLocation() - FVector(0.f, 0.f, 64.f));
	return Super::PreSpawnActor(Asset, InOutLocation);
}

FQuat UActorFactory_RuntimePCGVolume::AlignObjectToSurfaceNormal(const FVector& InSurfaceNormal, const FQuat& ActorRotation) const
{
	return FindActorAlignmentRotation(ActorRotation, FVector(0.f, 0.f, 1.f), InSurfaceNormal);
}

#undef LOCTEXT_NAMESPACE