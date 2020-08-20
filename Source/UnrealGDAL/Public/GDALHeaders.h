#pragma once

//Configure mergetiff to use Unreal Engine smart pointers
#include "Core/Public/Templates/UniquePtr.h"
#define MERGETIFF_SMART_POINTER_TYPE TUniquePtr
#define MERGETIFF_SMART_POINTER_GET(ptr) ptr.Get()
#define MERGETIFF_SMART_POINTER_RELEASE(ptr) ptr.Release()
#define MERGETIFF_SMART_POINTER_RESET(ptr, val) ptr.Reset(val)

//Configure mergetiff to not use exceptions
#include "Core/Public/CoreGlobals.h"
#include "Core/Public/Containers/StringConv.h"
#include "Core/Public/Logging/LogMacros.h"
#define MERGETIFF_DISABLE_EXCEPTIONS 1
#define MERGETIFF_ERROR_LOGGER(message) UE_LOG(LogTemp, Error, TEXT("%s"), UTF8_TO_TCHAR(message))

THIRD_PARTY_INCLUDES_START

#include <mergetiff/mergetiff.h>
#include <gdal.h>
#include <gdal_priv.h>
#include <gdal_utils.h>
#include <ogr_spatialref.h>
#include <ogrsf_frmts.h>
#include <cpl_conv.h>

THIRD_PARTY_INCLUDES_END
