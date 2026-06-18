#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LicenseBlueprintLibrary.generated.h"

UCLASS()
class LICENSEMANAGER_API ULicenseBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    /**
     * 检查 License 文件
     * 自动读取 exe 同级目录的 license.dat（编辑器下回退到 ProjectDir）
     *
     * @param OutExpireDate  - 输出：授权到期日期字符串（如 "2027-12-30"）
     * @param OutRawJson     - 输出：解密后的原始 JSON 字符串（调试用）
     * @param OutErrorMsg    - 输出：失败原因描述（调试用）
     * @return               - true = 授权有效且文件合法；false = 失败
     */
    UFUNCTION(BlueprintCallable, Category = "License", meta = (DisplayName = "Check License"))
    static bool CheckLicense(
        FString& OutExpireDate,
        FString& OutRawJson,
        FString& OutErrorMsg
    );

    /**
     * 检查 License 是否已过期
     * 需要先调用 CheckLicense 获得 ExpireDate
     *
     * @param ExpireDate  - 格式："YYYY-MM-DD"
     * @return            - true = 未过期；false = 已过期或格式错误
     */
    UFUNCTION(BlueprintCallable, Category = "License", meta = (DisplayName = "Is License Valid (Not Expired)"))
    static bool IsLicenseNotExpired(const FString& ExpireDate);

private:

    /**
     * 读取文件并用 AES-256-CBC 解密
     * 内部方法，蓝图不可见
     */
    static bool LoadAndDecryptCBC(
        const FString& FilePath,
        FString& OutJson,
        FString& OutError
    );

    /**
     * 手动实现 AES-256-CBC 解密
     * 因为 UE FAES::DecryptData 只做 ECB，我们手动 XOR 还原 CBC
     */
    static bool AES256CBC_Decrypt(
        const TArray<uint8>& CipherData,
        const uint8* Key,       // 32 bytes
        const uint8* IV,        // 16 bytes
        TArray<uint8>& OutPlain
    );
};