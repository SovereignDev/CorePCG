// © 2021, Brock Marsh. All rights reserved.


#include "Helpers/CorePCGStatics.h"

#include "PCGParamData.h"

UPCGMetadata* PCG::GetMetaData(UPCGData* Data)
{
	UPCGSpatialData* SpatialData = Cast<UPCGSpatialData>(Data);
	if(SpatialData) return SpatialData->MutableMetadata();

	UPCGParamData* ParamData = Cast<UPCGParamData>(Data);
	if(ParamData) return ParamData->MutableMetadata();

	return nullptr;
}
