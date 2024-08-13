// © 2021, Brock Marsh. All rights reserved.


#include "AssetTypes/RuntimePCG/Factory_RuntimePCG.h"

#include "Classes/RuntimePCGAsset.h"

UFactory_RuntimePCG::UFactory_RuntimePCG()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = URuntimePCGAsset::StaticClass();
}

UObject* UFactory_RuntimePCG::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	check(InClass->IsChildOf(URuntimePCGAsset::StaticClass()))
	
	URuntimePCGAsset* NewAsset = NewObject<URuntimePCGAsset>(InParent, InClass, InName, Flags | RF_Transactional);
	
	return NewAsset;
}