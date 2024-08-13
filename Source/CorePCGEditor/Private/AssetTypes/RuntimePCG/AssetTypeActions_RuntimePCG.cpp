// © 2021, Brock Marsh. All rights reserved.

#include "AssetTypes/RuntimePCG/AssetTypeActions_RuntimePCG.h"
#include "Classes/RuntimePCGAsset.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions_RuntimePCG"

FAssetTypeActions_RuntimePCG::FAssetTypeActions_RuntimePCG(EAssetTypeCategories::Type InAssetCategory)
: AssetCategory(InAssetCategory)
{}

FText FAssetTypeActions_RuntimePCG::GetName() const
{
	// Name of the Asset
	return LOCTEXT("AssetTypeActions_RuntimePCGName", "Runtime PCG Asset");
}

FColor FAssetTypeActions_RuntimePCG::GetTypeColor() const
{
	return FLinearColor(.35, 0.3, 1.0).ToFColor(true);
}

UClass* FAssetTypeActions_RuntimePCG::GetSupportedClass() const
{
	return URuntimePCGAsset::StaticClass();
}

void FAssetTypeActions_RuntimePCG::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	TArray<UObject*> ObjectsToEdit;
	TArray<UObject*> GraphsToEdit;
	ObjectsToEdit.Reserve(InObjects.Num());
	GraphsToEdit.Reserve(InObjects.Num());
	
	for (UObject* InObject : InObjects)
	{
		const URuntimePCGAsset* Asset = Cast<URuntimePCGAsset>(InObject);

		if(!Asset) continue;
		
		if (Asset->Graph)
		{
			GraphsToEdit.Add(Asset->Graph.Get());
		}
		else
		{
			ObjectsToEdit.Add(InObject);
		}
	}

	if(!ObjectsToEdit.IsEmpty()) FAssetTypeActions_Base::OpenAssetEditor(ObjectsToEdit, EditWithinLevelEditor);

	if(!GraphsToEdit.IsEmpty())
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		TWeakPtr<IAssetTypeActions> PCGActions = AssetTools.GetAssetTypeActionsForClass(UPCGGraph::StaticClass());

		PCGActions.Pin()->OpenAssetEditor(GraphsToEdit);
	}
}

uint32 FAssetTypeActions_RuntimePCG::GetCategories()
{
	return AssetCategory;
}

void FAssetTypeActions_RuntimePCG::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	FAssetTypeActions_Base::GetActions(InObjects, MenuBuilder);

	auto RuntimePCGAssets = GetTypedWeakObjectPtrs<URuntimePCGAsset>(InObjects);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("RuntimePCG_OpenAsset", "Edit Runtime PCG Asset"),
		LOCTEXT("RuntimePCG_OpenAssetTooltip", "Opens the Editor for the Runtime PCG Asset instead of the PCG Graph."),
		FSlateIcon( FAppStyle::GetAppStyleSetName(), "Icons.Edit" ),
		FUIAction(
			FExecuteAction::CreateSP(this, &FAssetTypeActions_RuntimePCG::OpenOriginalAsset, RuntimePCGAssets),
			FCanExecuteAction()
		)
	);
}

void FAssetTypeActions_RuntimePCG::OpenOriginalAsset(TArray<TWeakObjectPtr<URuntimePCGAsset>> Objects)
{
	TArray<UObject*> ObjectsToEdit;
	ObjectsToEdit.Reserve(Objects.Num());

	for (TWeakObjectPtr<URuntimePCGAsset> Object : Objects)
	{
		if(!Object.IsValid()) continue;
		
		ObjectsToEdit.Add(Object.Get());
	}

	FAssetTypeActions_Base::OpenAssetEditor(ObjectsToEdit);
}

#undef LOCTEXT_NAMESPACE
