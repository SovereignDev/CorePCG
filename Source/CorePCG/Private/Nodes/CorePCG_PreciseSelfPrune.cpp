// © 2021, Brock Marsh. All rights reserved.

#include "Nodes/CorePCG_PreciseSelfPrune.h"

#include "PCGContext.h"
#include "UDynamicMesh.h"
#include "GeometryScript/MeshComparisonFunctions.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"
#include "Helpers/CorePCGStatics.h"

#define LOCTEXT_NAMESPACE "PCGPreciseSelfPrune"

void AppendPointToMesh(UDynamicMesh* Mesh, const FPCGPoint& Point)
{
	FBox Box = Point.GetLocalBounds();

	// Scale the Box by the Point's Scale
	Box = Box.ExpandBy(Box.GetExtent() * (Point.Transform.GetScale3D() - FVector::One()));
	
	UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendBoundingBox(Mesh, FGeometryScriptPrimitiveOptions(), Point.Transform, Box);
}

bool IsIntersecting(UDynamicMesh* Mesh, UDynamicMesh* OtherMesh)
{
	bool bIsIntersecting = false;
	UGeometryScriptLibrary_MeshComparisonFunctions::IsIntersectingMesh(Mesh, FTransform::Identity, OtherMesh, FTransform::Identity, bIsIntersecting, nullptr);

	return bIsIntersecting;
}

bool FPCGPreciseSelfPruneElement::ExecuteInternal(FPCGContext* Context) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FPCGPreciseSelfPruneElement::Execute);

	if(!PCG::HasAnyInputs(Context)) return true;
	
	const UPCGPreciseSelfPruneSettings* Settings = Context->GetInputSettings<UPCGPreciseSelfPruneSettings>();
	check(Settings);

	// TODO COREPCG : Sort points based on pruning type
	const EPCGSelfPruningType PruningType = Settings->PruningType;
	
	TArray<FPCGTaggedData>& Outputs = Context->OutputData.TaggedData;
	const TArray<FPCGTaggedData> Inputs = Context->InputData.GetInputs();

	UDynamicMesh* FinalMesh = NewObject<UDynamicMesh>();
	UDynamicMesh* CurrentMesh = NewObject<UDynamicMesh>();
	
	ProcessPoints(Context, Inputs, Outputs, [FinalMesh, CurrentMesh](const FPCGPoint& InPoint, FPCGPoint& OutPoint)
	{
		OutPoint = InPoint;

		CurrentMesh->Reset();
		
		// Set the CurrentMesh to the Current Point
		AppendPointToMesh(CurrentMesh, OutPoint);

		// Test if the Final Merged Mesh is intersecting with the Current Mesh. If so return false so this point is not added to outputs.
		if(IsIntersecting(FinalMesh, CurrentMesh)) return false;
		
		// If not, append the Current Mesh to the Final Mesh
		AppendPointToMesh(FinalMesh, OutPoint);

		return true;
	});
	
	return true;
}