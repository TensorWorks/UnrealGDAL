#pragma once

#include "GDALHeaders.h"
#include "HelperStructs.h"
#include "SmartPointers.h"

#include "Core/Public/Math/Vector.h"
#include "Core/Public/Math/Vector2D.h"


//Boilerplate macro for declaring our convenience functions for parsing utility options
#define _UNREALGDAL_OPTS_PARSER_METHOD(dtype) static dtype##Ref Parse##dtype(const TArray<FString>& args);


//Provides helper functionality for working with the GDAL/OGR API
class UNREALGDAL_API GDALHelpers
{
	public:
		
		//------- DATASET ACCESS HELPERS -------
		
		//Attempts to open a raster dataset
		static GDALDatasetRef OpenRaster(const FString& DatasetPath, bool bReadOnly = true, const TArray<FString>& OpenOptions = {});
		
		//Attempts to open a vector dataset
		static GDALDatasetRef OpenVector(const FString& DatasetPath, bool bReadOnly = true, const TArray<FString>& OpenOptions = {});
		
		//Creates a unique `/vsimem/...` filename to be used when creating in-memory datasets (e.g. for passing to GDALTranslate())
		static FString UniqueMemFilename();
		
		
		//------- GEO-TRANSFORM HELPERS -------
		
		//Computes the corner coordinates of a GDAL dataset based on its geo-transform
		static RasterCornerCoordinatesRef GetRasterCorners(GDALDatasetRef& Dataset);
		
		//Sets the corner coordinates of a GDAL dataset, populating its geo-transform accordingly
		static bool SetRasterCorners(GDALDatasetRef& Dataset, const FVector2D& UpperLeft, const FVector2D& LowerRight);
		
		//Retrieves the geo-transform for a dataset
		static GeoTransformRef GetGeoTransform(GDALDatasetRef& Dataset);
		
		//Retrieves the geo-transform for a dataset and inverts it
		//(This is useful for converting from geo-coordinates to pixel coordinates in raster datasets)
		static GeoTransformRef GetInvertedGeoTransform(GDALDatasetRef& Dataset);
		
		//Applies a geo-transform to a point in 2D space
		static FVector2D ApplyGeoTransform(GeoTransformRef& Transform, const FVector2D& Point);
		static FVector2D ApplyGeoTransform(double* Transform, const FVector2D& Point);
		
		
		//------- SPATIAL REFERENCE SYSTEM AND COORDINATE TRANSFORMATION HELPERS -------
		
		//Retrieves the Well-Known Text (WKT) representation of the spatial reference system denoted by the specified EPSG identifier
		static FString WktFromEPSG(uint16 EPSG, bool bPretty = false);
		
		//Creates a coordinate transformation object for converting between two spatial reference systems
		static OGRCoordinateTransformationRef CreateCoordinateTransform(const FString& SourceWKT, const FString& TargetWKT);
		
		//Converts a coordinate using the supplied coordinate transformation object
		static bool TransformCoordinate(const OGRCoordinateTransformationRef& Transformation, const FVector& Input, FVector& Output);
		
		
		//------- RASTER MANIPULATION HELPERS -------
		
		//Computes the minimum and maximum values of the specified raster band of the supplied GDAL dataset
		//(Note that GDAL raster bands are 1-indexed rather than zero-indexed, so the index for the first band is 1)
		static RasterMinMaxRef ComputeRasterMinMax(GDALDatasetRef& Dataset, int BandIndex);
		
		//Allocates memory for raster data within an existing TArray and wraps it in a mergetiff RasterData object
		template <typename T>
		static mergetiff::RasterData<T> AllocateAndWrap(TArray<T>& Array, uint64 Channels, uint64 Rows, uint64 Cols, T Fill = T())
		{
			Array.Init(Fill, Channels * Rows * Cols);
			return mergetiff::RasterData<T>(Array.GetData(), Channels, Rows, Cols, true);
		}
		
		
		//------- GDAL UTILITY PROGRAM HELPERS -------
		
		//Unreal-friendly wrapper methods for parsing options for each of GDAL's utility programs
		_UNREALGDAL_OPTS_PARSER_METHOD(GDALInfoOptions)
		_UNREALGDAL_OPTS_PARSER_METHOD(GDALTranslateOptions)
		_UNREALGDAL_OPTS_PARSER_METHOD(GDALWarpAppOptions)
		_UNREALGDAL_OPTS_PARSER_METHOD(GDALVectorTranslateOptions)
		_UNREALGDAL_OPTS_PARSER_METHOD(GDALDEMProcessingOptions)
		_UNREALGDAL_OPTS_PARSER_METHOD(GDALNearblackOptions)
		_UNREALGDAL_OPTS_PARSER_METHOD(GDALGridOptions)
		_UNREALGDAL_OPTS_PARSER_METHOD(GDALRasterizeOptions)
		_UNREALGDAL_OPTS_PARSER_METHOD(GDALBuildVRTOptions)
		
		//Wrapper methods for GDAL utility programs introduced in GDAL 3.1
		#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,1,0)
		_UNREALGDAL_OPTS_PARSER_METHOD(GDALMultiDimInfoOptions)
		_UNREALGDAL_OPTS_PARSER_METHOD(GDALMultiDimTranslateOptions)
		#endif
		
		//Unreal-friendly wrappers for invoking common GDAL utility programs (currently just GDALTranslate())
		static GDALDatasetRef Translate(GDALDatasetRef& Source, const FString& Destination, const GDALTranslateOptionsRef& Options);
};
