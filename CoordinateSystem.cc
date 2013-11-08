#include "CoordinateSystem.hh"

#define _USE_MATH_DEFINES
#include <math.h>
#include <sstream>
#include <iostream>
#include <exception>

using std::string;
using std::stringstream;
using std::exception;


const double EARTH_RADIUS = 6378137.0;


sGeoCoord::sGeoCoord()
{
	sGeoCoord( 0.0, 0.0, COORDSYS_DDDDDDDD );
}



sGeoCoord::sGeoCoord( const double &lat, const double &lon,
					  const CoordinateSystem &system )
{
	latitude = lat;
	longitude = lon;
	coordSystem = system;
}



sGeoCoord sGeoCoord::To( const CoordinateSystem &system ) const
{
	if( system == coordSystem )
	{
		return GeoCoord( latitude, longitude, coordSystem );
	}

	if( latitude < 0 || longitude < 0 )
	{
		throw exception( "CoordinateSystem doesn't yet support negative values. Sorry." );
	}

	// Move the current coordinates to degree-system
	double lat;
	double lon;
	double degrees;
	double minutes;
	double seconds;


	switch( coordSystem )
	{
		case COORDSYS_DDDDDDDD:
			// Already in correct system
			lat = latitude;
			lon = longitude;
			break;

		case COORDSYS_DDMMMMMM:
			degrees = floor( latitude/100.0 );
			minutes = latitude - degrees*100.0;
			lat = degrees + minutes/60.0;

			degrees = floor( longitude/100.0 );
			minutes = longitude - degrees*100.0;
			lon = degrees + minutes/60.0;
			break;

		case COORDSYS_DDMMSSSS:
			throw exception( "Conversion from DDMMSSSS not implemented yet." );
			break;

		case COORDSYS_METERS:
			throw exception( "Conversion from meters to other systems is not supported. Yet." );
			break;

		default:
			throw exception( "Please, set the coordinate system to be something!" );
			break;
	}


	// Move to target system
	switch( system )
	{
		case CoordinateSystem::COORDSYS_DDDDDDDD:
			// Good, we don't need to do a thing
			break;

		case CoordinateSystem::COORDSYS_DDMMMMMM:
			degrees = floor( lat );
			minutes = (lat - degrees) * 60.0;
			lat = degrees*100.0 + minutes;

			degrees = floor( lon );
			minutes = (lon - degrees) * 60.0;
			lon = degrees*100.0 + minutes;
			break;

		case CoordinateSystem::COORDSYS_DDMMSSSS:
			throw exception( "Conversion to DDMMSSSS not implemented.." );
			break;

		case COORDSYS_METERS:
			throw exception( "Conversion from meters to other systems is not supported. Yet." );
			break;

		default:
			throw exception( "Please, set the coordinate system to be something!" );
			break;
	}

	return GeoCoord( lat, lon, system );
}



string sGeoCoord::String() const
{
	stringstream stream;
	stream << latitude << "," << longitude;
	return stream.str();
}



sGeoCoord sGeoCoord::Offset( const sGeoCoord &offset ) const
{
	GeoCoord pointA( latitude, longitude, coordSystem );
	GeoCoord pointB;

	double dLat; // Latitude difference, used when dealing with meters
	double dLon; // Longitude difference, same use.

	// Make sure we are in the correct system
	pointA = pointA.To( COORDSYS_DDDDDDDD );
	pointB.coordSystem = COORDSYS_DDDDDDDD;

	// Handle cases depending on the given offset's type
	switch( offset.coordSystem )
	{
		case COORDSYS_DDDDDDDD:
			pointB.latitude  = pointA.latitude  + offset.latitude;
			pointB.longitude = pointA.longitude + offset.longitude;
			break;

		case COORDSYS_METERS:
			// This requires math!
			dLat = offset.latitude / EARTH_RADIUS;
			dLon = offset.longitude / (EARTH_RADIUS * cos( M_PI * pointA.latitude / 180.0 ));
			pointB.latitude  = pointA.latitude  + dLat * 180.0/M_PI;
			pointB.longitude = pointA.longitude + dLon * 180.0/M_PI;
			break;

		default:
			throw exception( "Only coordinate offsets of types DDDDDDDD and METERS are accepted!" );
	}

	// Move the point back to the original system
	pointB = pointB.To( coordSystem );
	return pointB;
}
