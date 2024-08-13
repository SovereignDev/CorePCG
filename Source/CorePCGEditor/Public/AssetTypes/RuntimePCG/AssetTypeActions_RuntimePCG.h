// © 2021, Brock Marsh. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"

/**
 * 
 */
class COREPCGEDITOR_API FAssetTypeActions_RuntimePCG : public FAssetTypeActions_Base
{
public:
	FAssetTypeActions_RuntimePCG(EAssetTypeCategories::Type InAssetCategory);

	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;
	virtual uint32 GetCategories() override;

	virtual void GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder) override;

	void OpenOriginalAsset(TArray<TWeakObjectPtr<class URuntimePCGAsset>> Objects);

private:
	EAssetTypeCategories::Type AssetCategory;
};
