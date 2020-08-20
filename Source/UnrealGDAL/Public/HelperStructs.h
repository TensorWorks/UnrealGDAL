#pragma once

#include "Core/Public/Math/Vector2D.h"

//Represents the corner coordinates of a GDAL raster dataset
struct UNREALGDAL_API RasterCornerCoordinates
{
	FVector2D UpperLeft;
	FVector2D LowerRight;
};

//Represents the minimum and maximum values from a GDAL raster band
struct UNREALGDAL_API RasterMinMax
{
	RasterMinMax() : Min(INFINITY), Max(INFINITY) {}
	RasterMinMax(double Minimum, double Maximum) : Min(Minimum), Max(Maximum) {}
	
	double Min;
	double Max;
};
