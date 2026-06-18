#include "ConfigLibrary.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"

FString UConfigLibrary::GetConfigPath(const FString& FileName)
{
	FString BaseDir;

#if WITH_EDITOR
	// 编辑器模式：项目目录
	BaseDir = FPaths::ProjectDir();
#else
	// 打包模式：exe目录
	BaseDir = FPaths::LaunchDir();
#endif

	return FPaths::Combine(BaseDir, FileName);
}

FString UConfigLibrary::ReadConfig(const FString& FileName, const FString& Section, const FString& Key, const FString& DefaultValue)
{
	const FString FullPath = GetConfigPath(FileName);

	if (!GConfig)
	{
		return DefaultValue;
	}

	// 如果文件不存在，返回默认值
	if (!FPaths::FileExists(FullPath))
	{
		return DefaultValue;
	}

	// 重新加载配置文件
	GConfig->LoadFile(FullPath);

	FString Result;

	if (GConfig->GetString(*Section, *Key, Result, FullPath))
	{
		return Result;
	}

	return DefaultValue;
}

void UConfigLibrary::WriteConfig(const FString& FileName, const FString& Section, const FString& Key, const FString& Value)
{
	const FString FullPath = GetConfigPath(FileName);

	if (!GConfig)
	{
		return;
	}

	// 如果文件不存在则创建
	if (!FPaths::FileExists(FullPath))
	{
		FFileHelper::SaveStringToFile(TEXT(""), *FullPath);
	}

	// 写入配置
	GConfig->SetString(*Section, *Key, *Value, FullPath);

	// 保存到磁盘
	GConfig->Flush(false, FullPath);
}