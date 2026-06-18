#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WindowTool.generated.h"

//************************************************************
// 设置窗口或屏幕功能模块
//************************************************************

UCLASS()
class UWindowTool : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // 设置游戏窗口的位置，PosX 窗口左上角的X坐标，PosY 窗口左上角的Y坐标
    UFUNCTION(BlueprintCallable, Category = "SuperTool|Window", meta = (DisplayName = "Set Window Position"))
    static void SetGameWindowPosition(int32 PosX, int32 PosY);

    // 设置游戏窗口的大小,Width 窗口宽度,Height 窗口高度
    UFUNCTION(BlueprintCallable, Category = "SuperTool|Window", meta = (DisplayName = "Set Window Size"))
    static void SetGameWindowSize(int32 Width, int32 Height);

    // 获取窗口位置
    UFUNCTION(BlueprintCallable, Category = "SuperTool|Window", meta = (DisplayName = "Get Window Position"))
    static FVector2D GetGameWindowPosition();

    // 获取窗口大小
    UFUNCTION(BlueprintCallable, Category = "SuperTool|Window", meta = (DisplayName = "Get Window Size"))
    static FVector2D GetGameWindowSize();
};
