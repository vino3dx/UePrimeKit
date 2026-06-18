#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ConfigLibrary.generated.h"

UCLASS()
class SUPERTOOL_API UConfigLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    // 读取INI配置到字符串
    UFUNCTION(BlueprintCallable, Category = "SuperTool|Config",
        meta = (ToolTip = "从可执行目录读取INI配置文件", CPP_Default_FileName = "Config.ini"))
    static FString ReadConfig
    (
        UPARAM(DisplayName = "FileName") const FString& FileName,
        UPARAM(DisplayName = "Section") const FString& Section,
        UPARAM(DisplayName = "Key") const FString& Key,
        UPARAM(DisplayName = "DefaultValue") const FString& DefaultValue
    );

    // 写入INI配置到字符串
    UFUNCTION(BlueprintCallable, Category = "SuperTool|Config",
        meta = (ToolTip = "将配置写入可执行目录的INI文件", CPP_Default_FileName = "Config.ini"))
    static void WriteConfig
    (
        UPARAM(DisplayName = "FileName") const FString& FileName,
        UPARAM(DisplayName = "Section") const FString& Section,
        UPARAM(DisplayName = "Key") const FString& Key,
        UPARAM(DisplayName = "Value") const FString& Value
    );

private:

    // 获取配置文件完整路径（自动适配Editor/Shipping）
    static FString GetConfigPath(const FString& FileName);
};