// © 2021, Brock Marsh. All rights reserved.


#include "Nodes/CorePCG_PerlinNoise2D.h"

#include "PCGContext.h"

UPCGPerlinNoise2DSettings::UPCGPerlinNoise2DSettings()
{
	bUseSeed = true;
}

bool FPCGPerlinNoise2DElement::ExecuteInternal(FPCGContext* Context) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FPCGPerlinNoise2DElement::Execute);

	const UPCGPerlinNoise2DSettings* Settings = Context->GetInputSettings<UPCGPerlinNoise2DSettings>();
	check(Settings);

	TArray<FPCGTaggedData> Inputs = Context->InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel);
	TArray<FPCGTaggedData>& Outputs = Context->OutputData.TaggedData;

	float Scale = Settings->Scale;
	float Frequency = Settings->Frequency * 0.001f;
	float Power = Settings->Power;
	
	int32 Seed = Settings->GetSeed(Context->SourceComponent.Get());

	ProcessPoints(Context, Inputs, Outputs,[&Scale, &Frequency, &Power, &Seed](const FPCGPoint& InPoint, FPCGPoint& OutPoint)->bool
	{
		OutPoint = InPoint;

		FVector Location(InPoint.Transform.GetLocation());
		
		Location += FVector(Seed);

		Location *= Frequency;
		
		float Noise = FMath::PerlinNoise2D(FVector2D(Location));

		Noise += 1.f;
		Noise /= 2.f;

		Noise = FMath::Pow(Noise, Power);
		Noise *= Scale;

		OutPoint.Density = Noise;

		return true;
	});

	return true;
}