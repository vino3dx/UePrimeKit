// ============================================================
// LicenseBlueprintLibrary.cpp
// UE5 AES-256-CBC License 验证插件（修正版）
// ============================================================

#include "LicenseBlueprintLibrary.h"

// UE Core
#include "Misc/FileHelper.h"
#include "Misc/Base64.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "HAL/PlatformFilemanager.h"
#include "HAL/PlatformFile.h"

// JSON
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

// AES（UE5 正确头文件）
#include "Misc/AES.h"

// ============================================================
// 固定密钥（必须与 Python 完全一致）
// AES-256 = 32 bytes
// IV = 16 bytes
// ============================================================

static const uint8 LICENSE_AES_KEY[32] =
{
    '1','2','3','4','5','6','7','8','9','0',
    'A','B','C','D','E','F',
    '1','2','3','4','5','6','7','8','9','0',
    'A','B','C','D','E','F'
};

static const uint8 LICENSE_AES_IV[16] =
{
    'A','B','C','D','E','F',
    '1','2','3','4','5','6','7','8','9','0'
};

// ============================================================
// AES-256-CBC 解密（FAES + 手动 CBC）
// ============================================================

bool ULicenseBlueprintLibrary::AES256CBC_Decrypt(
    const TArray<uint8>& CipherData,
    const uint8* Key,
    const uint8* IV,
    TArray<uint8>& OutPlain
)
{
    const int32 BlockSize = 16;

    if (CipherData.Num() <= 0 || CipherData.Num() % BlockSize != 0)
    {
        UE_LOG(LogTemp, Error, TEXT("[LicenseManager] Invalid cipher size: %d"), CipherData.Num());
        return false;
    }

    FAES::FAESKey AESKey;
    FMemory::Memcpy(AESKey.Key, Key, 32);

    const int32 NumBlocks = CipherData.Num() / BlockSize;
    OutPlain.SetNumUninitialized(CipherData.Num());

    for (int32 i = 0; i < NumBlocks; i++)
    {
        const uint8* InBlock = CipherData.GetData() + i * BlockSize;
        uint8* OutBlock = OutPlain.GetData() + i * BlockSize;

        uint8 TempBlock[16];
        FMemory::Memcpy(TempBlock, InBlock, BlockSize);

        // ECB decrypt
        FAES::DecryptData(TempBlock, BlockSize, AESKey);

        const uint8* XorSrc =
            (i == 0) ? IV : (CipherData.GetData() + (i - 1) * BlockSize);

        for (int32 b = 0; b < BlockSize; b++)
        {
            OutBlock[b] = TempBlock[b] ^ XorSrc[b];
        }
    }

    // PKCS7 Padding 去除
    if (OutPlain.Num() > 0)
    {
        uint8 PadLen = OutPlain.Last();

        if (PadLen < 1 || PadLen > BlockSize)
        {
            UE_LOG(LogTemp, Error, TEXT("[LicenseManager] Invalid padding: %d"), PadLen);
            return false;
        }

        for (int32 i = 0; i < PadLen; i++)
        {
            if (OutPlain[OutPlain.Num() - 1 - i] != PadLen)
            {
                UE_LOG(LogTemp, Error, TEXT("[LicenseManager] Padding check failed"));
                return false;
            }
        }

        OutPlain.SetNum(OutPlain.Num() - PadLen);
    }

    return true;
}

// ============================================================
// 文件读取 + Base64 + 解密
// ============================================================

bool ULicenseBlueprintLibrary::LoadAndDecryptCBC(
    const FString& FilePath,
    FString& OutJson,
    FString& OutError
)
{
    FString Base64Text;

    if (!FFileHelper::LoadFileToString(Base64Text, *FilePath))
    {
        OutError = FString::Printf(TEXT("Cannot read file: %s"), *FilePath);
        return false;
    }

    Base64Text = Base64Text.TrimStartAndEnd();
    Base64Text = Base64Text.Replace(TEXT("\r"), TEXT(""));
    Base64Text = Base64Text.Replace(TEXT("\n"), TEXT(""));
    Base64Text = Base64Text.Replace(TEXT(" "), TEXT(""));

    if (Base64Text.IsEmpty())
    {
        OutError = TEXT("Base64 is empty");
        return false;
    }

    TArray<uint8> EncryptedData;

    if (!FBase64::Decode(Base64Text, EncryptedData))
    {
        OutError = TEXT("Base64 decode failed");
        return false;
    }

    TArray<uint8> PlainData;

    if (!AES256CBC_Decrypt(EncryptedData, LICENSE_AES_KEY, LICENSE_AES_IV, PlainData))
    {
        OutError = TEXT("AES decrypt failed");
        return false;
    }

    if (PlainData.Num() == 0)
    {
        OutError = TEXT("Empty decrypted data");
        return false;
    }

    PlainData.Add(0);

    FUTF8ToTCHAR Conv(reinterpret_cast<const ANSICHAR*>(PlainData.GetData()));
    OutJson = FString(Conv.Length(), Conv.Get());

    return true;
}

// ============================================================
// License Check 主入口（Blueprint）
// ============================================================

bool ULicenseBlueprintLibrary::CheckLicense(
    FString& OutExpireDate,
    FString& OutRawJson,
    FString& OutErrorMsg
)
{
    OutExpireDate.Empty();
    OutRawJson.Empty();
    OutErrorMsg.Empty();

    TArray<FString> Paths;

    FString LaunchDir = FPaths::LaunchDir();
    Paths.Add(LaunchDir / TEXT("license.dat"));

    FString ProjectDir = FPaths::ProjectDir();
    Paths.Add(ProjectDir / TEXT("license.dat"));

    Paths.Add(FPaths::ProjectContentDir() / TEXT("license.dat"));

    FString FoundPath;

    for (const FString& P : Paths)
    {
        if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*P))
        {
            FoundPath = P;
            break;
        }
    }

    if (FoundPath.IsEmpty())
    {
        OutErrorMsg = TEXT("license.dat not found");
        return false;
    }

    FString JsonStr;
    FString Err;

    if (!LoadAndDecryptCBC(FoundPath, JsonStr, Err))
    {
        OutErrorMsg = Err;
        return false;
    }

    OutRawJson = JsonStr;

    TSharedPtr<FJsonObject> JsonObj;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonStr);

    if (!FJsonSerializer::Deserialize(Reader, JsonObj) || !JsonObj.IsValid())
    {
        OutErrorMsg = TEXT("JSON parse failed");
        return false;
    }

    if (!JsonObj->HasField(TEXT("expire")))
    {
        OutErrorMsg = TEXT("Missing expire field");
        return false;
    }

    OutExpireDate = JsonObj->GetStringField(TEXT("expire"));

    if (!IsLicenseNotExpired(OutExpireDate))
    {
        OutErrorMsg = TEXT("License expired");
        return false;
    }

    return true;
}

// ============================================================
// 过期判断
// ============================================================

bool ULicenseBlueprintLibrary::IsLicenseNotExpired(const FString& ExpireDate)
{
    TArray<FString> Parts;
    ExpireDate.ParseIntoArray(Parts, TEXT("-"));

    if (Parts.Num() != 3)
        return false;

    FDateTime Expire(
        FCString::Atoi(*Parts[0]),
        FCString::Atoi(*Parts[1]),
        FCString::Atoi(*Parts[2]),
        23, 59, 59
    );

    return FDateTime::Now() <= Expire;
}