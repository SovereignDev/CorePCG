// © 2021, Brock Marsh. All rights reserved.


#include "Nodes/CorePCG_AssignSingleMesh.h"

#include "PCGComponent.h"
#include "PCGContext.h"
#include "PCGPoint.h"
#include "Data/PCGSpatialData.h"
#include "Helpers/CorePCGStatics.h"
#include "Helpers/PCGHelpers.h"
#include "Metadata/PCGMetadataAccessor.h"

UPCGAssignSingleMeshSettings::UPCGAssignSingleMeshSettings()
{
	bUseSeed = false;
}

bool FPCGAssignSingleMeshElement::AsyncExecuteInternal(FCorePCGAsyncContext* Context) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FPCGAssignSingleMeshElement::Execute);

	const UPCGAssignSingleMeshSettings* Settings = Context->GetInputSettings<UPCGAssignSingleMeshSettings>();
	check(Settings);

	const TArray<FPCGTaggedData> InputsToCheckIfEmpty = Context->InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel);
	
	/* Dont Generate if no inputs */
	if(!InputsToCheckIfEmpty.IsValidIndex(0)) return false;

	TSoftObjectPtr<UStaticMesh> Mesh = Settings->Mesh;
	
	/* Dont modify if no mesh is specified */
	if(Mesh.IsNull())
	{
		PCGE_LOG(Warning, GraphAndLog, FText::FromString("Assign Single Mesh Node has no mesh specified!"));
		return true;
	}

	bool bSetBounds = Settings->bSetBounds;
	float BoundsScale = Settings->BoundsScale;

	// Before Returning, Load all the Chosen Meshes Asyncronously.
	CorePCGAsyncLoadHelpers::RequestAsyncLoad(Mesh, FStreamableDelegate::CreateLambda([this, Context, Mesh, bSetBounds, BoundsScale]
	{
		TArray<FPCGTaggedData> Inputs = Context->InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel);
		TArray<FPCGTaggedData>& Outputs = Context->OutputData.TaggedData;
	
		// If Inputs Changed during Async Load then Cancel
		if(!Inputs.IsValidIndex(0))
		{
			FinishExecute(Context);
			return;
		}
		
		UPCGSpatialData* SpatialData = Cast<UPCGSpatialData>(Inputs[0].Data);
		if(!SpatialData)
		{
			FinishExecute(Context);
			return;
		}

		UPCGMetadata* Metadata = SpatialData->MutableMetadata();

		if(!Metadata->HasAttribute(FName("Mesh"))) Metadata->CreateStringAttribute(FName("Mesh"), "", false, true);
		
		// Process Points after Async Load because we need to gather the Meshes Bounds.
		PCG::ProcessPointsSynchronous(Context, Inputs, Outputs, [Mesh, &bSetBounds, &BoundsScale, Metadata](FPCGPoint& Point)
		{
			UPCGMetadataAccessorHelpers::SetStringAttribute(Point, Metadata, FName("Mesh"), Mesh.ToSoftObjectPath().GetAssetPathString());
	
			if(bSetBounds)
			{
				Point.BoundsMin = Mesh.LoadSynchronous()->GetBounds().GetBox().Min * BoundsScale;
				Point.BoundsMax = Mesh.LoadSynchronous()->GetBounds().GetBox().Max * BoundsScale;
			}
	
			return true;
		});

		for (FPCGTaggedData& Output : Outputs) Cast<UPCGSpatialData>(Output.Data)->Metadata = Metadata;
		
		// Finish the Async Execute so the Graph can continue.
		FinishExecute(Context);
	}));
	
	return true;
}