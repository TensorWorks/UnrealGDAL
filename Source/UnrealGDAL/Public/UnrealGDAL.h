#pragma once

#include "CoreMinimal.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "GDALHeaders.h"

class UNREALGDAL_API FUnrealGDALModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	//Configures the GDAL data directory and registers all GDAL drivers
	//This function must be called by any module that consumes the GDAL/OGR API and must be inlined under Windows due to DLL boundary issues
	FORCEINLINE void InitGDAL()
	{
		//Point GDAL to the path containing its runtime data files
		FString dataDir = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir(), TEXT("Binaries"), TEXT("Data"), TEXT("GDAL")));
		CPLSetConfigOption("GDAL_DATA", TCHAR_TO_UTF8(*dataDir));
		
		//Register all GDAL format drivers
		GDALAllRegister();
		
		//Install our GDAL error handler
		CPLSetErrorHandler(FUnrealGDALModule::GDALErrorHandler);
	}
	
	//Our custom error handler for logging errors emitted by GDAL
	static void GDALErrorHandler(CPLErr err, int num, const char* message);
};
