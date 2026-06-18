using UnrealBuildTool;

public class SuperTool : ModuleRules
{
    public SuperTool(ReadOnlyTargetRules Target) : base(Target)
    {
        // PCH 使用模式
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // 公共依赖模块（蓝图节点、运行时都可以使用）
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",         // UE核心
            "CoreUObject",  // UObject系统
            "Engine",       // 引擎核心
            "Slate",        // UI框架
            "SlateCore",    // UI核心模块
            "RHI",          // 渲染硬件接口
            "RenderCore",   // 渲染核心模块
            "ImageWrapper", // 图片编码/解码（PNG JPG等）
            "Networking",   // 网络模块（注意大小写）
            "Sockets"       // Socket通信（UDP/TCP）
        });

        // 私有依赖模块（只在CPP内部使用）
        PrivateDependencyModuleNames.AddRange(new string[]
        {

        });
    }
}