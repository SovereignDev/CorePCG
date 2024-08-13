// © 2021, Brock Marsh. All rights reserved.


#include "Nodes/CorePCG_AssignActor.h"

#include "PCGContext.h"
#include "PCGModule.h"
#include "Data/PCGSpatialData.h"
#include "Engine/AssetManager.h"
#include "Helpers/CorePCGMetadata.h"
#include "Helpers/CorePCGRandomHelpers.h"
#include "Helpers/PCGHelpers.h"
#include "Metadata/PCGMetadataAccessor.h"

bool FPCGAssignActorElement::AsyncExecuteInternal(FCorePCGAsyncContext* Context) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FPCGAssignActorElement::Execute);

	const UPCGAssignActorSettings* Settings = Context->GetInputSettings<UPCGAssignActorSettings>();
	check(Settings);

	TArray<FPCGTaggedData> Inputs = Context->InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel);
	TArray<FPCGTaggedData>& Outputs = Context->OutputData.TaggedData;
	
	/* Dont Generate if no inputs */
	if(!Inputs.IsValidIndex(0)) return false;

	TArray<FWeightedActorToPoint> Meshes = Settings->Actors;

	/* Dont modify if no meshes are specified */
	if(Meshes.IsEmpty())
	{
		PCGE_LOG(Warning, GraphAndLog, FText::FromString("Assign Actor Node has no Actors specified!"));
		return false;
	}

	bool bSetBounds = Settings->bSetBounds;
	float BoundsScale = Settings->BoundsScale;

	int32 Seed = Context->GetSeed();

	const UPCGSpatialData* SpatialData = Cast<UPCGSpatialData>(Inputs[0].Data);
	if(!SpatialData) return false;

	// Store an Array of the Classes that we want to spawn so we can async load them.
	TArray<TSoftClassPtr<AActor>> ClassesToLoad;
	
	UPCGMetadata* Metadata = const_cast<UPCGSpatialData*>(SpatialData)->MutableMetadata();
	
	if(!Metadata->HasAttribute(FName("ActorClass"))) Metadata->CreateStringAttribute(FName("ActorClass"), "", false, true);

	ProcessPoints(Context, Inputs, Outputs, [&Meshes, &ClassesToLoad, &bSetBounds, &BoundsScale, &Metadata, Seed](const FPCGPoint& InPoint, FPCGPoint& OutPoint)
	{
		OutPoint = InPoint;

		const FRandomStream RandomStream = PCGHelpers::ComputeSeed(Seed, InPoint.Seed);

		const FWeightedActorToPoint ChosenActor = GET_FROM_WEIGHT_STRUCT_WITH_STREAM(FWeightedActorToPoint, Meshes, RandomStream);

		ClassesToLoad.AddUnique(ChosenActor.Class);

		CorePCGMetaData::SetAttribute(OutPoint, Metadata, FName("ActorClass"), ChosenActor.Class.ToSoftObjectPath().GetAssetPathString());

		if(bSetBounds)
		{
			OutPoint.BoundsMin = ChosenActor.Bounds.Min * BoundsScale;
			OutPoint.BoundsMax = ChosenActor.Bounds.Max * BoundsScale;
		}

		return true;
	});

	for (FPCGTaggedData& Output : Outputs) const_cast<UPCGSpatialData*>(Cast<UPCGSpatialData>(Output.Data))->Metadata = Metadata;

	// Before Returning, Load all the Chosen Meshes Asyncronously.
	CorePCGAsyncLoadHelpers::RequestAsyncLoad(ClassesToLoad, FStreamableDelegate::CreateLambda([Context]
	{
		// Finish the Async Execute so the Graph can continue.
		FinishExecute(Context);
	}));
	
	return true;
}

#if WITH_EDITOR
void UPCGAssignActorSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Check if we need to recalculate the bounds of any of the actors
	if(PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, Actors)
		|| PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(FWeightedActorToPoint, Class)
		|| PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, bCalculateBoundsFromCollision))
	{
		// Create a transient world so we can spawn the actors
		TSharedPtr<FPreviewScene> PreviewScene = MakeShared<FPreviewScene>(FPreviewScene::ConstructionValues());
		
		for (FWeightedActorToPoint& Actor : Actors)
		{
			if(Actor.Class.IsNull()) continue;

			TSubclassOf<AActor> ActorClass = Actor.Class.LoadSynchronous();
			if(!ActorClass) continue;

			const AActor* SpawnedActor = PreviewScene->GetWorld()->SpawnActor(ActorClass);

			FVector Origin;
			FVector BoxExtent;
			SpawnedActor->GetActorBounds(bCalculateBoundsFromCollision, Origin, BoxExtent);
			
			Actor.Bounds = FBox::BuildAABB(Origin, BoxExtent);
		}

		PreviewScene = nullptr;
	}
}
#endif