#include "GDALHelpers.h"
#include "Misc/Guid.h"

GDALDatasetRef GDALHelpers::OpenRaster(const FString& DatasetPath, bool bReadOnly, const TArray<FString>& OpenOptions)
{
	//If an array of options were specified to pass to the GDAL driver then process them
	mergetiff::ArgsArray options;
	for (FString option : OpenOptions) {
		options.add(TCHAR_TO_UTF8(*option));
	}
	
	//Attempt to open the specified dataset
	return GDALDatasetRef((GDALDataset*)GDALOpenEx(
		TCHAR_TO_UTF8(*DatasetPath),
		GDAL_OF_RASTER | GDAL_OF_VERBOSE_ERROR | (bReadOnly ? GDAL_OF_READONLY : 0),
		nullptr,
		(OpenOptions.Num() > 0) ? options.get() : nullptr,
		nullptr
	));
}

GDALDatasetRef GDALHelpers::OpenVector(const FString& DatasetPath, bool bReadOnly, const TArray<FString>& OpenOptions)
{
	//If an array of options were specified to pass to the GDAL driver then process them
	mergetiff::ArgsArray options;
	for (FString option : OpenOptions) {
		options.add(TCHAR_TO_UTF8(*option));
	}
	
	//Attempt to open the specified dataset
	return GDALDatasetRef((GDALDataset*)GDALOpenEx(
		TCHAR_TO_UTF8(*DatasetPath),
		GDAL_OF_VECTOR | GDAL_OF_VERBOSE_ERROR | (bReadOnly ? GDAL_OF_READONLY : 0),
		nullptr,
		(OpenOptions.Num() > 0) ? options.get() : nullptr,
		nullptr
	));
}

FString GDALHelpers::UniqueMemFilename() {
	return FString::Printf(TEXT("/vsimem/%s.tif"), *(FGuid::NewGuid().ToString()));
}

RasterCornerCoordinatesRef GDALHelpers::GetRasterCorners(GDALDatasetRef& Dataset)
{
	//Verify that a valid dataset was supplied
	if (!Dataset) {
		return RasterCornerCoordinatesRef(nullptr);
	}
	
	//Retrieve the geo-transform for the dataset
	GeoTransformRef transform = GDALHelpers::GetGeoTransform(Dataset);
	if (!transform) {
		return RasterCornerCoordinatesRef(nullptr);
	}
	
	//Retrieve the dataset's raster dimensions
	int xSize = Dataset->GetRasterXSize();
	int ySize = Dataset->GetRasterYSize();
	
	//Compute the corner coordinates for the dataset
	//(Based on the inverse of the calculation we perform in GDALHelpers::SetRasterCorners() below)
	RasterCornerCoordinatesRef corners(new RasterCornerCoordinates());
	corners->UpperLeft.X = transform[0];
	corners->UpperLeft.Y = transform[3];
	corners->LowerRight.X = corners->UpperLeft.X + (transform[1] * xSize);
	corners->LowerRight.Y = corners->UpperLeft.Y + (transform[5] * ySize);
	return corners;
}

bool GDALHelpers::SetRasterCorners(GDALDatasetRef& Dataset, const FVector2D& UpperLeft, const FVector2D& LowerRight)
{
	//Verify that a valid dataset was supplied
	if (!Dataset) {
		return false;
	}
	
	//Retrieve the dataset's raster dimensions and create a geo-transform to be populated
	int xSize = Dataset->GetRasterXSize();
	int ySize = Dataset->GetRasterYSize();
	double transform[6];
	
	//Populate the geo-transform
	//(Based directly on the calculation performed by GDALTranslate():
	// <https://github.com/OSGeo/gdal/blob/v3.1.2/gdal/apps/gdal_translate_lib.cpp#L1369-L1379>)
	transform[0] = UpperLeft.X;
	transform[1] = (LowerRight.X - UpperLeft.X) / xSize;
	transform[2] = 0.0;
	transform[3] = UpperLeft.Y;
	transform[4] = 0.0;
	transform[5] = (LowerRight.Y - UpperLeft.Y) / ySize;
	
	//Attempt to apply the populate geo-transform
	return (Dataset->SetGeoTransform(transform) == CE_None);
}

GeoTransformRef GDALHelpers::GetGeoTransform(GDALDatasetRef& Dataset)
{
	//Verify that a valid dataset was supplied
	if (!Dataset) {
		return GeoTransformRef(nullptr);
	}
	
	//Attempt to retrieve the geo-transform from the dataset
	GeoTransformRef transform(new double[6]);
	if (Dataset->GetGeoTransform(transform.Get()) == CE_Failure) {
		return GeoTransformRef(nullptr);
	}
	
	return transform;
}

GeoTransformRef GDALHelpers::GetInvertedGeoTransform(GDALDatasetRef& Dataset)
{
	//Attempt to retrieve the geo-transform from the dataset
	GeoTransformRef transform = GDALHelpers::GetGeoTransform(Dataset);
	if (!transform) {
		return GeoTransformRef(nullptr);
	}
	
	//Attempt to invert the geo-transform
	GeoTransformRef inverted(new double[6]);
	if (!GDALInvGeoTransform(transform.Get(), inverted.Get())) {
		return GeoTransformRef(nullptr);
	}
	
	return inverted;
}

FVector2D GDALHelpers::ApplyGeoTransform(GeoTransformRef& Transform, const FVector2D& Point) {
	return GDALHelpers::ApplyGeoTransform(Transform.Get(), Point);
}

FVector2D GDALHelpers::ApplyGeoTransform(double* Transform, const FVector2D& Point)
{
	double x, y;
	GDALApplyGeoTransform(Transform, Point.X, Point.Y, &x, &y);
	return FVector2D(x, y);
}

FString GDALHelpers::WktFromEPSG(uint16 EPSG, bool bPretty)
{
	//Attempt to create the specified spatial reference system
	OGRSpatialReference srs;
	if (srs.importFromEPSG(EPSG) != OGRERR_NONE) {
		return TEXT("");
	}
	
	//Attempt to export the WKT for the spatial reference system
	char* wkt = nullptr;
	OGRErr err = (bPretty) ? srs.exportToPrettyWkt(&wkt) : srs.exportToWkt(&wkt);
	if (err != OGRERR_NONE) {
		return TEXT("");
	}
	
	//Convert the WKT string to an FString and free the memory allocated by GDAL
	CPLStringRef string(wkt);
	return UTF8_TO_TCHAR(string.Get());
}

OGRCoordinateTransformationRef GDALHelpers::CreateCoordinateTransform(const FString& SourceWKT, const FString& TargetWKT)
{
	//Attempt to create the source coordinate system
	OGRSpatialReference source;
	if (source.importFromWkt(TCHAR_TO_UTF8(*SourceWKT)) != OGRERR_NONE) {
		return OGRCoordinateTransformationRef(nullptr);
	}
	
	//Attempt to create target coordinate system
	OGRSpatialReference target;
	if (target.importFromWkt(TCHAR_TO_UTF8(*TargetWKT)) != OGRERR_NONE) {
		return OGRCoordinateTransformationRef(nullptr);
	}
	
	//Create the coordinate transformation object
	return OGRCoordinateTransformationRef(OGRCreateCoordinateTransformation(&source, &target));
}

bool GDALHelpers::TransformCoordinate(const OGRCoordinateTransformationRef& Transformation, const FVector& Input, FVector& Output)
{
	//Unpack the input coordinate vector
	double x = Input.X;
	double y = Input.Y;
	double z = Input.Z;
	
	//Attempt to perform the conversion (will modify the values in-place)
	if (Transformation && Transformation->Transform(1, &x, &y, &z))
	{
		Output.X = (float)x;
		Output.Y = (float)y;
		Output.Z = (float)z;
		return true;
	}
	
	//The conversion failed
	return false;
}

RasterMinMaxRef GDALHelpers::ComputeRasterMinMax(GDALDatasetRef& Dataset, int BandIndex)
{
	//Verify that a valid dataset was supplied
	if (!Dataset) {
		return RasterMinMaxRef(nullptr);
	}
	
	//Attempt to retrieve the specified raster band
	GDALRasterBand* band = Dataset->GetRasterBand(BandIndex);
	if (band == nullptr) {
		return RasterMinMaxRef(nullptr);
	}
	
	//Attempt to compute the minimum and maximum values
	double minMax[2];
	if (band->ComputeRasterMinMax(0, minMax) != CE_None) {
		return RasterMinMaxRef(nullptr);
	}
	
	return RasterMinMaxRef(new RasterMinMax(minMax[0], minMax[1]));
}

namespace
{
	template<typename T, T(*ParserFunc)(mergetiff::ArgsArray&)>
	T OptionsParserBase(const TArray<FString>& Args)
	{
		//Convert the supplied arguments into an argv-style structure
		mergetiff::ArgsArray argvArray;
		for (FString arg : Args) {
			argvArray.add(TCHAR_TO_UTF8(*arg));
		}
		
		//Call the underlying parser function from mergetiff
		return ParserFunc(argvArray);
	}
}

#define _UNREALGDAL_OPTS_PARSER_IMPLEMENTATION(dtype) dtype##Ref GDALHelpers::Parse##dtype(const TArray<FString>& Args) { \
	return OptionsParserBase<dtype##Ref,mergetiff::OptionsParsing::parse##dtype>(Args); \
}

_UNREALGDAL_OPTS_PARSER_IMPLEMENTATION(GDALInfoOptions)
_UNREALGDAL_OPTS_PARSER_IMPLEMENTATION(GDALTranslateOptions)
_UNREALGDAL_OPTS_PARSER_IMPLEMENTATION(GDALWarpAppOptions)
_UNREALGDAL_OPTS_PARSER_IMPLEMENTATION(GDALVectorTranslateOptions)
_UNREALGDAL_OPTS_PARSER_IMPLEMENTATION(GDALDEMProcessingOptions)
_UNREALGDAL_OPTS_PARSER_IMPLEMENTATION(GDALNearblackOptions)
_UNREALGDAL_OPTS_PARSER_IMPLEMENTATION(GDALGridOptions)
_UNREALGDAL_OPTS_PARSER_IMPLEMENTATION(GDALRasterizeOptions)
_UNREALGDAL_OPTS_PARSER_IMPLEMENTATION(GDALBuildVRTOptions)

#if GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(3,1,0)
_UNREALGDAL_OPTS_PARSER_IMPLEMENTATION(GDALMultiDimInfoOptions)
_UNREALGDAL_OPTS_PARSER_IMPLEMENTATION(GDALMultiDimTranslateOptions)
#endif

GDALDatasetRef GDALHelpers::Translate(GDALDatasetRef& Source, const FString& Destination, const GDALTranslateOptionsRef& Options)
{
	return GDALDatasetRef((GDALDataset*)GDALTranslate(
		TCHAR_TO_UTF8(*Destination),
		Source.Get(),
		Options.Get(),
		nullptr
	));
}
