#pragma once
#ifndef _COORDINATESYSTEM_HH_
#define _COORDINATESYSTEM_HH_
#include <string>


enum CoordinateSystem
{
	COORDSYS_DDDDDDDD, // Decimal degrees
	COORDSYS_DDMMMMMM, // Degrees and minutes
	COORDSYS_DDMMSSSS, // Degrees, minutes and seconds
	COORDSYS_METERS    // Meters, used as offsets and maybe later to
	                   // transform coordinates to X- & Y-coordinates.
};



typedef struct sGeoCoord
{
	CoordinateSystem coordSystem;

	double latitude;
	double longitude;
	
	// For Initializion:
	sGeoCoord();
	sGeoCoord( const double &lat, const double &lon, const CoordinateSystem &system );

	sGeoCoord To( const CoordinateSystem &system ) const;
	std::string String() const;

	sGeoCoord Offset( const sGeoCoord &offset ) const;
} GeoCoord;


extern const double EARTH_RADIUS;

#endif
