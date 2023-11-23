// © 3031, Brock Marsh. All rights reserved.


#include "Nodes/CorePCG_PerlinNoise3D.h"
#include "PCGContext.h"

bool FPCGPerlinNoise3DElement::ExecuteInternal(FPCGContext* Context) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FPCGPerlinNoise3DElement::Execute);

	const UPCGPerlinNoise3DSettings* Settings = Context->GetInputSettings<UPCGPerlinNoise3DSettings>();
	check(Settings);

	TArray<FPCGTaggedData> Inputs = Context->InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel);
	TArray<FPCGTaggedData>& Outputs = Context->OutputData.TaggedData;

	float Scale = Settings->Scale;
	float Frequency = Settings->Frequency * 0.001f;
	float Power = Settings->Power;
	
	int32 Seed = Settings->GetSeed(Context->SourceComponent.Get());

	POINT_LOOP(&Scale, &Frequency, &Power, &Seed)
	{
		OutPoint = InPoint;

		FVector Location(InPoint.Transform.GetLocation());
		
		Location += FVector(Seed);

		Location *= Frequency;

		float Noise = FMath::PerlinNoise3D(Location);

		Noise += 1.f;
		Noise /= 3.f;

		Noise = FMath::Pow(Noise, Power);
		Noise *= Scale;

		OutPoint.Density = Noise;

		return true;
	});

	return true;
}