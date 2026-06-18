#include "FileTool.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "Modules/ModuleManager.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Engine/Texture2D.h"
#include "Rendering/Texture2DResource.h"
#include "Engine/TextureDefines.h"

static FString GetAbsolutePath(const FString& Relative)
{
    return FPaths::Combine(FPaths::ProjectDir(), Relative);
}


/*-----------------------------------------------------------
    图片加载
-----------------------------------------------------------*/

UTexture2D* UFileTool::LoadTextureFromFile(const FString& FilePath)
{
    if (!FPaths::FileExists(FilePath))
    {
        return nullptr;
    }

    TArray<uint8> FileData;
    if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
    {
        return nullptr;
    }

    IImageWrapperModule& ImageModule =
        FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");

    EImageFormat Format = ImageModule.DetectImageFormat(FileData.GetData(), FileData.Num());
    if (Format == EImageFormat::Invalid)
    {
        return nullptr;
    }

    TSharedPtr<IImageWrapper> Wrapper = ImageModule.CreateImageWrapper(Format);

    if (!Wrapper.IsValid())
    {
        return nullptr;
    }

    if (!Wrapper->SetCompressed(FileData.GetData(), FileData.Num()))
    {
        return nullptr;
    }

    TArray<uint8> RawData;

    if (!Wrapper->GetRaw(ERGBFormat::BGRA, 8, RawData))
    {
        return nullptr;
    }

    int32 Width = Wrapper->GetWidth();
    int32 Height = Wrapper->GetHeight();

    UTexture2D* Texture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);

    if (!Texture)
    {
        return nullptr;
    }

    Texture->CompressionSettings = TC_VectorDisplacementmap;
    Texture->SRGB = true;
    Texture->SetFlags(RF_Transient);

    FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];

    void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);

    FMemory::Memcpy(Data, RawData.GetData(), RawData.Num());

    Mip.BulkData.Unlock();

    Texture->UpdateResource();

    return Texture;
}


/*-----------------------------------------------------------
    单图加载
-----------------------------------------------------------*/

UTexture2D* UFileTool::LoadSingleExternalImageAsTexture(const FString& RelativeFilePath)
{
    const FString AbsoluteFilePath = GetAbsolutePath(RelativeFilePath);

    return LoadTextureFromFile(AbsoluteFilePath);
}


/*-----------------------------------------------------------
    批量图片
-----------------------------------------------------------*/

TArray<UTexture2D*> UFileTool::LoadAllExternalImages(const FString& RelativeFolder)
{
    TArray<UTexture2D*> Textures;

    const FString Folder = GetAbsolutePath(RelativeFolder);

    TArray<FString> ImagePaths;

    const TArray<FString> Extensions =
    {
        TEXT("*.png"),
        TEXT("*.jpg"),
        TEXT("*.jpeg")
    };

    for (const FString& Ext : Extensions)
    {
        TArray<FString> Temp;

        IFileManager::Get().FindFilesRecursive(
            Temp,
            *Folder,
            *Ext,
            true,
            false
        );

        ImagePaths.Append(Temp);
    }

    for (const FString& File : ImagePaths)
    {
        if (UTexture2D* Tex = LoadTextureFromFile(File))
        {
            Textures.Add(Tex);
        }
    }

    return Textures;
}


/*-----------------------------------------------------------
    读取TXT
-----------------------------------------------------------*/

FString UFileTool::ReadText(const FString& Path)
{
    FString Result;

    const FString FullPath = GetAbsolutePath(Path);

    if (FPaths::FileExists(FullPath))
    {
        FFileHelper::LoadFileToString(Result, *FullPath);
    }

    return Result;
}


/*-----------------------------------------------------------
    批量TXT
-----------------------------------------------------------*/

TArray<FString> UFileTool::LoadAllExternalTexts(const FString& RelativeFolder)
{
    TArray<FString> Texts;

    const FString Folder = GetAbsolutePath(RelativeFolder);

    TArray<FString> TxtPaths;

    IFileManager::Get().FindFilesRecursive(
        TxtPaths,
        *Folder,
        TEXT("*.txt"),
        true,
        false
    );

    for (const FString& File : TxtPaths)
    {
        FString Content;

        if (FFileHelper::LoadFileToString(Content, *File))
        {
            Texts.Add(Content);
        }
    }

    return Texts;
}


/*-----------------------------------------------------------
    图片 + 文本
-----------------------------------------------------------*/

TArray<FDataStructure> UFileTool::LoadAllExternalImagesAndTexts(const FString& RelativeFolder)
{
    TArray<FDataStructure> DataStructures;

    const FString Folder = GetAbsolutePath(RelativeFolder);

    TMap<FString, UTexture2D*> ImageMap;
    TMap<FString, FString> TextMap;

    /* 图片 */

    TArray<FString> ImagePaths;

    const TArray<FString> Extensions =
    {
        TEXT("*.png"),
        TEXT("*.jpg"),
        TEXT("*.jpeg")
    };

    for (const FString& Ext : Extensions)
    {
        TArray<FString> Temp;

        IFileManager::Get().FindFilesRecursive(
            Temp,
            *Folder,
            *Ext,
            true,
            false
        );

        ImagePaths.Append(Temp);
    }

    for (const FString& Path : ImagePaths)
    {
        FString Name = FPaths::GetBaseFilename(Path);

        if (UTexture2D* Tex = LoadTextureFromFile(Path))
        {
            ImageMap.Add(Name, Tex);
        }
    }

    /* 文本 */

    TArray<FString> TxtPaths;

    IFileManager::Get().FindFilesRecursive(
        TxtPaths,
        *Folder,
        TEXT("*.txt"),
        true,
        false
    );

    for (const FString& Path : TxtPaths)
    {
        FString Name = FPaths::GetBaseFilename(Path);

        FString Content;

        if (FFileHelper::LoadFileToString(Content, *Path))
        {
            TextMap.Add(Name, Content);
        }
    }

    /* 合并 */

    TSet<FString> AllKeys;

    ImageMap.GetKeys(AllKeys);
    TextMap.GetKeys(AllKeys);

    for (const FString& Key : AllKeys)
    {
        FDataStructure Entry;

        Entry.Title = Key;
        Entry.Content = TextMap.FindRef(Key);
        Entry.Image = ImageMap.FindRef(Key);

        DataStructures.Add(Entry);
    }

    return DataStructures;
}