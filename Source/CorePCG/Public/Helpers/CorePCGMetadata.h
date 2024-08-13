// © 2021, Brock Marsh. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Metadata/PCGMetadata.h"
#include "Metadata/PCGMetadataAttributeTpl.h"
#include "Metadata/PCGMetadataAttributeTraits.h"


namespace CorePCGMetaData
{
	template<typename T>
	void SetAttribute(FPCGPoint& Point, UPCGMetadata* Metadata, FName AttributeName, const T& Value)
	{
		if (!Metadata) return;

		PCGMetadataEntryKey Key = Point.MetadataEntry;

		Metadata->InitializeOnSet(Key);
		
		if (Key == PCGInvalidEntryKey)
		{
			UE_LOG(LogPCG, Error, TEXT("[PCG] Metadata key has no entry, therefore can't set values."))
			return;
		}
		
		FPCGMetadataAttribute<T>* Attribute = static_cast<FPCGMetadataAttribute<T>*>(Metadata->GetMutableAttribute(AttributeName));
		if (Attribute && Attribute->GetTypeId() == PCG::Private::MetadataTypes<T>::Id)
		{
			Attribute->SetValue(Key, Value);
		}
	}
	
	template<typename T>
	T GetAttribute(const FPCGPoint& Point, const UPCGMetadata* Metadata, FName AttributeName)
	{
		if (!Metadata) return T{};

		PCGMetadataEntryKey Key = Point.MetadataEntry;

		const FPCGMetadataAttributeBase* AttributeBase = Metadata->GetConstAttribute(AttributeName);
		if (!AttributeBase) return T{};

		if (AttributeBase->GetTypeId() == PCG::Private::MetadataTypes<T>::Id)
		{
			return static_cast<const FPCGMetadataAttribute<T>*>(AttributeBase)->GetValueFromItemKey(Key);
		}
		else if constexpr (std::is_same_v<T, FString>)
		{
			// Legacy path - allow reading soft object/class paths using string accessor.
			if (AttributeBase->GetTypeId() == PCG::Private::MetadataTypes<FSoftObjectPath>::Id)
			{
				return static_cast<const FPCGMetadataAttribute<FSoftObjectPath>*>(AttributeBase)->GetValueFromItemKey(Key).ToString();
			}
			else if (AttributeBase->GetTypeId() == PCG::Private::MetadataTypes<FSoftClassPath>::Id)
			{
				return static_cast<const FPCGMetadataAttribute<FSoftClassPath>*>(AttributeBase)->GetValueFromItemKey(Key).ToString();
			}
		}

		return T{};
	}
}