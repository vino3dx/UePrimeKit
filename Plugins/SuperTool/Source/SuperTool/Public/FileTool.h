#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/Texture2D.h"
#include "FileTool.generated.h"

/**
 * 图片 + 文本 数据结构
 */
USTRUCT(BlueprintType)
struct FDataStructure
{
    GENERATED_BODY()

public:

    FDataStructure()
        : Title(TEXT(""))
        , Content(TEXT(""))
        , Image(nullptr)
    {
    }

    /** 标题（通常为文件名） */
    UPROPERTY(BlueprintReadWrite, Category = "SuperTool|File")
    FString Title = TEXT("");

    /** 文本内容 */
    UPROPERTY(BlueprintReadWrite, Category = "SuperTool|File")
    FString Content = TEXT("");

    /** 关联图片 */
    UPROPERTY(BlueprintReadWrite, Category = "SuperTool|File")
    TObjectPtr<UTexture2D> Image = nullptr;
};


/**
 * 文件读取工具库
 */
UCLASS()
class UFileTool : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    /** 读取TXT文件 */
    UFUNCTION(BlueprintCallable, Category = "SuperTool|File")
    static FString ReadText(const FString& Path);

    /** 读取文件夹内所有TXT */
    UFUNCTION(BlueprintCallable, Category = "SuperTool|File")
    static TArray<FString> LoadAllExternalTexts(const FString& RelativeFolder);

    /** 从磁盘加载图片 */
    UFUNCTION(BlueprintCallable, Category = "SuperTool|File")
    static UTexture2D* LoadTextureFromFile(const FString& FilePath);

    /** 加载单张图片 */
    UFUNCTION(BlueprintCallable, Category = "SuperTool|File")
    static UTexture2D* LoadSingleExternalImageAsTexture(const FString& RelativeFilePath);

    /** 加载所有图片 */
    UFUNCTION(BlueprintCallable, Category = "SuperTool|File")
    static TArray<UTexture2D*> LoadAllExternalImages(const FString& RelativeFolder);

    /** 加载图片 + TXT */
    UFUNCTION(BlueprintCallable, Category = "SuperTool|File")
    static TArray<FDataStructure> LoadAllExternalImagesAndTexts(const FString& RelativeFolder);
};