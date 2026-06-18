#include "UDP.h"
#include "SocketSubsystem.h"
#include "Common/UdpSocketBuilder.h"

AUDP::AUDP()
{
	// 禁用 Tick，提高性能
	PrimaryActorTick.bCanEverTick = false;
}

void AUDP::BeginPlay()
{
	Super::BeginPlay();
}

void AUDP::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 结束时关闭接收器，释放资源
	StopUDPReceiver();
	Super::EndPlay(EndPlayReason);
}

/* ================= 接收器 ================= */

bool AUDP::StartUDPReceiver(const FString& SocketName, int32 ListenPort)
{
	// 如果已有监听，先关闭
	if (ListenSocket)
	{
		StopUDPReceiver();
	}

	// 绑定到指定端口
	FIPv4Endpoint Endpoint(FIPv4Address::Any, ListenPort);

	// 创建 UDP Socket
	ListenSocket = FUdpSocketBuilder(*SocketName)
		.AsNonBlocking() // 非阻塞
		.AsReusable() // 可复用
		.BoundToEndpoint(Endpoint) // 绑定端口
		.WithReceiveBufferSize(2 * 1024 * 1024); // 接收缓冲区大小

	if (!ListenSocket)
	{
		return false;
	}

	bIsListening = true;
	return true;
}

void AUDP::StopUDPReceiver()
{
	// 关闭并销毁 Socket
	if (ListenSocket)
	{
		ListenSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)
			->DestroySocket(ListenSocket);
		ListenSocket = nullptr;
	}

	bIsListening = false;
}

bool AUDP::ReceiveRaw(TArray<uint8>& OutData)
{
	if (!ListenSocket) return false;

	uint8 Buffer[65507]; // UDP 最大报文长度

	int32 BytesRead = 0;

	// 获取发送方地址
	TSharedRef<FInternetAddr> Sender =
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

	// 从 Socket 接收数据
	if (ListenSocket->RecvFrom(Buffer, sizeof(Buffer), BytesRead, *Sender))
	{
		if (BytesRead > 0)
		{
			OutData.SetNum(BytesRead);
			FMemory::Memcpy(OutData.GetData(), Buffer, BytesRead);
			return true;
		}
	}

	return false;
}

/* ================= 公共接收接口 ================= */

bool AUDP::ReceiveUDP(EUDPDataFormat DataFormat, FString& OutString)
{
	TArray<uint8> RawData;
	if (!ReceiveRaw(RawData)) return false;

	// 转换为字符串
	OutString = BytesToString(RawData, DataFormat);
	return true;
}

/* ================= 公共发送接口 ================= */

bool AUDP::SendUDP(
	const FString& Message,
	const FString& TargetIP,
	int32 TargetPort,
	EUDPDataFormat DataFormat)
{
	// 字符串转字节数组
	TArray<uint8> Payload;
	if (!StringToBytes(Message, DataFormat, Payload))
	{
		return false;
	}

	ISocketSubsystem* SS = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	if (!SS) return false;

	// 解析目标 IP
	FIPv4Address Addr;
	if (!FIPv4Address::Parse(TargetIP, Addr)) return false;

	TSharedRef<FInternetAddr> Dest = SS->CreateInternetAddr();
	Dest->SetIp(Addr.Value);
	Dest->SetPort(TargetPort);

	// 创建发送 Socket
	FSocket* SendSocket = FUdpSocketBuilder(TEXT("UDP_Send"))
		.AsNonBlocking()
		.AsReusable();

	if (!SendSocket) return false;

	// 发送数据
	int32 BytesSent = 0;
	bool bOK = SendSocket->SendTo(
		Payload.GetData(),
		Payload.Num(),
		BytesSent,
		*Dest
	);

	// 关闭并销毁发送 Socket
	SendSocket->Close();
	SS->DestroySocket(SendSocket);

	return bOK;
}

/* ================= 数据转换 ================= */

FString AUDP::BytesToString(
	const TArray<uint8>& Data,
	EUDPDataFormat Format) const
{
	switch (Format)
	{
	case EUDPDataFormat::UTF8:
	{
		// UTF8 转 FString
		FUTF8ToTCHAR Convert(
			reinterpret_cast<const ANSICHAR*>(Data.GetData()),
			Data.Num());

		return FString(Convert.Length(), Convert.Get());
	}

	case EUDPDataFormat::ANSI:
	case EUDPDataFormat::ASCII:
	{
		// ANSI/ASCII 转 FString
		return FString(
			StringCast<TCHAR>(
				reinterpret_cast<const ANSICHAR*>(Data.GetData()),
				Data.Num()).Get());
	}

	case EUDPDataFormat::HEX:
	{
		// 转十六进制字符串
		FString Hex;
		for (uint8 B : Data)
		{
			Hex += FString::Printf(TEXT("%02X "), B);
		}
		return Hex.TrimEnd();
	}
	}

	return TEXT("");
}

bool AUDP::StringToBytes(
	const FString& InString,
	EUDPDataFormat Format,
	TArray<uint8>& OutData) const
{
	switch (Format)
	{
	case EUDPDataFormat::UTF8:
	{
		// FString 转 UTF8
		FTCHARToUTF8 Conv(*InString);
		OutData.Append(
			reinterpret_cast<const uint8*>(Conv.Get()),
			Conv.Length());
		return true;
	}

	case EUDPDataFormat::ANSI:
	case EUDPDataFormat::ASCII:
	{
		// FString 转 ANSI/ASCII
		auto Converted = StringCast<ANSICHAR>(*InString);
		OutData.Append(
			reinterpret_cast<const uint8*>(Converted.Get()),
			Converted.Length());
		return true;
	}

	case EUDPDataFormat::HEX:
	{
		TArray<FString> Parts;
		InString.ParseIntoArrayWS(Parts);

		for (const FString& P : Parts)
		{
			uint8 Byte = FParse::HexDigit(P[0]) << 4 |
				FParse::HexDigit(P[1]);
			OutData.Add(Byte);
		}
		return true;
	}
	}

	return false;
}
