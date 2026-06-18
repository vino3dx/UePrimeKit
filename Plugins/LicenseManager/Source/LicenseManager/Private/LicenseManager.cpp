#include "LicenseManager.h"

#define LOCTEXT_NAMESPACE "FLicenseManagerModule"

void FLicenseManagerModule::StartupModule()
{
    UE_LOG(LogTemp, Log, TEXT("[LicenseManager] Module Started"));
}

void FLicenseManagerModule::ShutdownModule()
{
    UE_LOG(LogTemp, Log, TEXT("[LicenseManager] Module Shutdown"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FLicenseManagerModule, LicenseManager)