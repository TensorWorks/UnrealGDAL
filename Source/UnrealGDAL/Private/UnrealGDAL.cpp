#include "UnrealGDAL.h"
#include "GDALHelpers.h"

#define LOCTEXT_NAMESPACE "FUnrealGDALModule"

void FUnrealGDALModule::StartupModule() {
	this->InitGDAL();
}

void FUnrealGDALModule::ShutdownModule() {}

void FUnrealGDALModule::GDALErrorHandler(CPLErr err, int num, const char* message) {
	UE_LOG(LogTemp, Error, TEXT("%s"), UTF8_TO_TCHAR(message))
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FUnrealGDALModule, UnrealGDAL)