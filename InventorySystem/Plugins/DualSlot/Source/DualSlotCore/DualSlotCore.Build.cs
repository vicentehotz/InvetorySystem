// Copyright Under the Bay Studios. All Rights Reserved.

using UnrealBuildTool;

public class DualSlotCore : ModuleRules
{
	public DualSlotCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"NetCore"
			}
		);
	}
}
