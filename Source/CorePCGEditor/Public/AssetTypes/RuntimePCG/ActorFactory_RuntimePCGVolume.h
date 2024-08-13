// © 2021, Brock Marsh. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ActorFactories/ActorFactoryBoxVolume.h"
#include "ActorFactory_RuntimePCGVolume.generated.h"

/**
 * 
 */
UCLASS()
class COREPCGEDITOR_API UActorFactory_RuntimePCGVolume : public UActorFactoryBoxVolume
{
	GENERATED_BODY()
	
	UActorFactory_RuntimePCGVolume(const FObjectInitializer& ObjectInitializer);

public:
	//~ Begin UActorFactory Interface
	virtual bool CanCreateActorFrom( const FAssetData& AssetData, FText& OutErrorMsg ) override;
	virtual UClass* GetDefaultActorClass(const FAssetData& AssetData) override;
	virtual void PostPlaceAsset(TArrayView<const FTypedElementHandle> InElementHandles, const FAssetPlacementInfo& InPlacementInfo, const FPlacementOptions& InPlacementOptions) override;
	virtual UObject* GetAssetFromActorInstance(AActor* ActorInstance) override;
	virtual bool PreSpawnActor(UObject* Asset, FTransform& InOutLocation) override;
	virtual FQuat AlignObjectToSurfaceNormal(const FVector& InSurfaceNormal, const FQuat& ActorRotation) const override;
	//~ End UActorFactory Interface
};