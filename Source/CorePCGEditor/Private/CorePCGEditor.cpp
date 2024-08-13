#include "CorePCGEditor.h"

#include "IAssetTools.h"
#include "AssetTypes/RuntimePCG/AssetTypeActions_RuntimePCG.h"

#define LOCTEXT_NAMESPACE "FCorePCGEditorModule"

void FCorePCGEditorModule::StartupModule()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	CategoryBit = AssetTools.FindAdvancedAssetCategory("PCG");
	
	RegisterAssetTypeAction(AssetTools, MakeShareable(new FAssetTypeActions_RuntimePCG(CategoryBit)));
}

void FCorePCGEditorModule::ShutdownModule()
{
    
}

void FCorePCGEditorModule::RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action)
{
	AssetTools.RegisterAssetTypeActions(Action);
	CreatedAssetTypeActions.Add(Action);
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FCorePCGEditorModule, CorePCGEditor)