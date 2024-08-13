using UnrealBuildTool;

public class CorePCGEditor : ModuleRules
{
    public CorePCGEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core"
                , "CorePCG"
                , "AssetTools"
                , "UnrealEd"
                , "PCG"
                
                , "TypedElementFramework"
                , "TypedElementRuntime"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore"
                , "PCGEditor"
            }
        );
    }
}