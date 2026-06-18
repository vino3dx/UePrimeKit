#include "WindowTool.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Slate/SceneViewport.h"
#include "Widgets/SWindow.h"

//================================================================================
// 设置窗口位置和尺寸大小
//================================================================================
// 设置窗口位置
void UWindowTool::SetGameWindowPosition(int32 PosX, int32 PosY)
{
    if (GEngine && GEngine->GameViewport)
    {
        TSharedPtr<SWindow> Window = GEngine->GameViewport->GetWindow();
        if (Window.IsValid())
        {
            // 移动窗口到指定位置
            Window->MoveWindowTo(FVector2D(PosX, PosY));
            UE_LOG(LogTemp, Log, TEXT("窗口已移动到位置 (%d, %d)"), PosX, PosY);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("无效的游戏窗口。"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("引擎或 GameViewport 不可用。"));
    }
}

// 设置窗口大小
void UWindowTool::SetGameWindowSize(int32 Width, int32 Height)
{
    if (GEngine && GEngine->GameViewport)
    {
        TSharedPtr<SWindow> Window = GEngine->GameViewport->GetWindow();
        if (Window.IsValid())
        {
            // 调整窗口大小
            Window->Resize(FVector2D(Width, Height));
            UE_LOG(LogTemp, Log, TEXT("窗口大小已调整为宽: %d, 高: %d"), Width, Height);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("无效的游戏窗口。"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("引擎或 GameViewport 不可用。"));
    }
}

// 获取窗口位置
FVector2D UWindowTool::GetGameWindowPosition()
{
    if (GEngine && GEngine->GameViewport)
    {
        TSharedPtr<SWindow>Window = GEngine->GameViewport->GetWindow();
        if (Window.IsValid())
        {
            return Window->GetPositionInScreen();
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("无法获取窗口位置"));
    return FVector2D::ZeroVector;
}

// 获取窗口大小
FVector2D UWindowTool::GetGameWindowSize()
{
    if (GEngine && GEngine->GameViewport)
    {
        TSharedPtr<SWindow>Window = GEngine->GameViewport->GetWindow();
        if (Window.IsValid())
        {
            return Window->GetSizeInScreen();
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("无法获取窗口大小"));
    return FVector2D::ZeroVector;
}
