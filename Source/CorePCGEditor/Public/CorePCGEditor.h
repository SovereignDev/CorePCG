#pragma once

#include "CoreMinimal.h"
#include "AssetTypeCategories.h"
#include "IAssetTools.h"
#include "IAssetTypeActions.h"
#include "Modules/ModuleManager.h"


class FCorePCGEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    void RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action);

    TArray< TSharedPtr<IAssetTypeActions> > CreatedAssetTypeActions;

    EAssetTypeCategories::Type CategoryBit = EAssetTypeCategories::None;
};
