// © 2021, Brock Marsh. All rights reserved.

#include "Nodes/CorePCG_AssignMesh.h"

#include "PCGComponent.h"
#include "PCGContext.h"
#include "PCGPoint.h"
#include "Data/PCGSpatialData.h"
#include "Helpers/CorePCGMetadata.h"
#include "Helpers/CorePCGRandomHelpers.h"
#include "Helpers/CorePCGStatics.h"
#include "Helpers/PCGHelpers.h"
#include "Metadata/PCGMetadataAccessor.h"

#if WITH_EDITOR
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#endif

UPCGAssignMeshSettings::UPCGAssignMeshSettings()
{
	bUseSeed = true;
}

FPCGElementPtr UPCGAssignMeshSettings::CreateElement() const
{
	return MakeShared<FPCGAssignMeshElement>();
}

#if WITH_EDITOR
void UPCGAssignMeshSettings::AddSelectedMeshes()
{
	Modify();
	
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
    TArray<FAssetData> SelectedAssets;
    ContentBrowserModule.Get().GetSelectedAssets(SelectedAssets);

	for (FAssetData& Asset : SelectedAssets)
	{
		UStaticMesh* Mesh = Cast<UStaticMesh>(Asset.GetAsset());

		if(!Mesh) continue;
		if(Meshes.Contains(Mesh)) continue;

		FWeightedMeshToPoint NewMesh;
		NewMesh.Mesh = Mesh;
		NewMesh.Bounds = Mesh->GetBounds().GetBox();

		Meshes.Add(NewMesh);
	}
}

void UPCGAssignMeshSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if(PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, Meshes)
		|| PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(FWeightedMeshToPoint, Mesh))
	{
		for (FWeightedMeshToPoint& Mesh : Meshes)
		{
			if(Mesh.Mesh.IsNull()) continue;

			Mesh.Bounds = Mesh.Mesh.LoadSynchronous()->GetBounds().GetBox();
		}
	}
}
#endif

bool FPCGAssignMeshElement::AsyncExecuteInternal(FCorePCGAsyncContext* Context) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FPCGAssignMeshElement::Execute);

	const UPCGAssignMeshSettings* Settings = Context->GetInputSettings<UPCGAssignMeshSettings>();
	check(Settings);
	
	TArray<FPCGTaggedData> Inputs = Context->InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel);
	TArray<FPCGTaggedData>& Outputs = Context->OutputData.TaggedData;
	
	/* Dont Generate if no inputs */
	if(!Inputs.IsValidIndex(0)) return false;

	TArray<FWeightedMeshToPoint> Meshes = Settings->Meshes;
	
	/* Dont modify if no meshes are specified */
	if(Meshes.IsEmpty())
	{
		PCGE_LOG(Warning, GraphAndLog, FText::FromString("Assign Mesh Node has no meshes specified!"));
		return true;
	}

	bool bSetBounds = Settings->bSetBounds;
	float BoundsScale = Settings->BoundsScale;
	
	int32 Seed = Context->GetSeed();

	UPCGSpatialData* SpatialData = const_cast<UPCGSpatialData*>(Cast<UPCGSpatialData>(Inputs[0].Data));
	if(!SpatialData) return false;

	UPCGMetadata* Metadata = SpatialData->MutableMetadata();

	// Create an Array of All Chosen Meshes so we can Async Load them.
	TArray<TSoftObjectPtr<UStaticMesh>> MeshesToLoad;
	
	if(!Metadata->HasAttribute(FName("Mesh"))) Metadata->CreateStringAttribute(FName("Mesh"), "", false, true);
	
	PCG::ProcessPointsSynchronous(Context, Inputs, Outputs, [Context, &Meshes, &MeshesToLoad, &bSetBounds, &BoundsScale, &Metadata, Seed](FPCGPoint& OutPoint)
	{
		// Check if this Node was Cancelled.
		if(!Context) return false;
		
		const FRandomStream RandomStream = PCGHelpers::ComputeSeed(Seed, OutPoint.Seed);
	
		const FWeightedMeshToPoint ChosenMesh = GET_FROM_WEIGHT_STRUCT_WITH_STREAM(FWeightedMeshToPoint, Meshes, RandomStream);
		
		MeshesToLoad.AddUnique(ChosenMesh.Mesh);
			
		CorePCGMetaData::SetAttribute(OutPoint, Metadata, FName("Mesh"), ChosenMesh.Mesh.ToSoftObjectPath().GetAssetPathString());
		
		if(bSetBounds)
		{
			OutPoint.BoundsMin = ChosenMesh.Bounds.Min * BoundsScale;
			OutPoint.BoundsMax = ChosenMesh.Bounds.Max * BoundsScale;
		}
	
		return true;
	});

	for (FPCGTaggedData& Output : Outputs) const_cast<UPCGSpatialData*>(Cast<UPCGSpatialData>(Output.Data))->Metadata = Metadata;
	
	// Before Returning, Load all the Chosen Meshes Asyncronously.
	CorePCGAsyncLoadHelpers::RequestAsyncLoad(MeshesToLoad, FStreamableDelegate::CreateLambda([Context]
	{
		// Finish the Async Execute so the Graph can continue.
		FinishExecute(Context);
	}));
	
	return true;
}