using UnrealBuildTool;

public class GeneratorDungeon : ModuleRules
{
    public GeneratorDungeon(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "GameCore",
        });

        PrivateDependencyModuleNames.AddRange(new string[] { });
    }
}
