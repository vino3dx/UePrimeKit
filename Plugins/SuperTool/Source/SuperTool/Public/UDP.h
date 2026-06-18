#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sockets.h"
#include "UDP.generated.h"
/* udp数据格式 */
UENUM(BlueprintType)
enum class EUDPDataFormat : uint8
{
	UTF8  UMETA(DisplayName = "UTF-8 String"),
	ANSI  UMETA(DisplayName = "ANSI String"),
	ASCII UMETA(DisplayName = "ASCII String"),
	HEX   UMETA(DisplayName = "Hex String")
};

UCLASS()
class SUPERTOOL_API AUDP : public AActor
{
	GENERATED_BODY()

public:
	AUDP();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    // 是否正在监听
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SuperTool|UDP")
    bool bIsListening = false;

    // 接收 Socket
    FSocket* ListenSocket = nullptr;

    // 启动接收器
    UFUNCTION(BlueprintCallable, Category = "SuperTool|UDP")
    bool StartUDPReceiver(const FString& SocketName, int32 ListenPort);

    // 停止接收器
    UFUNCTION(BlueprintCallable, Category = "SuperTool|UDP")
    void StopUDPReceiver();

    // 接收数据并转为字符串
    UFUNCTION(BlueprintCallable, Category = "SuperTool|UDP")
    bool ReceiveUDP(EUDPDataFormat DataFormat, FString& OutString);

    // 发送字符串数据
    UFUNCTION(BlueprintCallable, Category = "SuperTool|UDP")
    bool SendUDP(const FString& Message, const FString& TargetIP, int32 TargetPort, EUDPDataFormat DataFormat);

private:
	// 原始字节接收
	bool ReceiveRaw(TArray<uint8>& OutData);

	// 字节转字符串
	FString BytesToString(const TArray<uint8>& Data, EUDPDataFormat Format) const;

	// 字符串转字节
	bool StringToBytes(const FString& InString, EUDPDataFormat Format, TArray<uint8>& OutData) const;
};
