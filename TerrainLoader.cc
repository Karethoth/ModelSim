#include "TerrainLoader.hh"
#include "libs/vjson/json.h"

#include <curl/curl.h>
#include <string>
#include <sstream>
#include <math.h>
#include <OGRE\Ogre.h>
#include <OGRE\OgreVector3.h>

#define _USE_MATH_DEFINES
#include <math.h>


// The base URL used for accessing Google's Elevation API
static const std::string baseURL = "http://maps.googleapis.com/maps/api/elevation/json?sensor=false&locations=";



// This needs to be rethought, it is just terrible.
static std::string curlBuffer;
static size_t data_write( void* buf, size_t size, size_t nmemb, void* userp )
{
    size_t realsize = size * nmemb;
    curlBuffer.append( (char*)buf, realsize );
    return realsize;
}



TerrainLoader::TerrainLoader( SimApp &app ) : app( app )
{
	finished = false;
	referenceRatio = 1.0;
}



void TerrainLoader::LoadArea( const GeoCoord &center,
							  const double &degsLat,
							  const double &degsLon,
							  const unsigned char &lod,
							  const std::string &areaName )
{
	// Create list of all points in the grid
	std::vector<GeoCoord> gridPoints;
	GeoCoord currentPoint;

	// Calculate area limits
	double leftLongitude = center.longitude - degsLon;
	double rightLongitude = center.longitude + degsLon;
	double topLatitude = center.latitude + degsLat;
	double bottomLatitude = center.latitude -degsLat;

	// Calculate latitude and longitude step sizes
	double stepLatitude  = (topLatitude-bottomLatitude)   / lod;
	double stepLongitude = (rightLongitude-leftLongitude) / lod;


	for( unsigned int y=0; y <= lod; ++y )
	{
		currentPoint = GeoCoord( topLatitude - stepLatitude*y, leftLongitude, COORDSYS_DDDDDDDD );

		for( unsigned int x=0; x <= lod; ++x )
		{
			gridPoints.push_back( currentPoint );
			currentPoint = currentPoint.Offset( GeoCoord( 0.0, stepLongitude, COORDSYS_DDDDDDDD ) );
		}
	}


	// Get elevation data for the points in the grid:
	std::vector<ElevationInfo> *elevationData;
	elevationData = LoadElevationData( gridPoints );
	
	// Calculate latitude degrees to meter ratio
	double latitudeMeterRatio  = referencePoint.Offset( GeoCoord( 1.0, 0.0, COORDSYS_METERS ) ).latitude -
			                     referencePoint.latitude;

	// Generate the terrain mesh
	Ogre::ManualObject *terrainObject = nullptr;
	terrainObject = app.sceneManager->createManualObject( Ogre::String( areaName ) );
	terrainObject->setDynamic( false );
	terrainObject->begin( "BaseWhiteNoLighting", Ogre::RenderOperation::OT_TRIANGLE_LIST );
	{
		unsigned int index;

		// Push vertex data
		std::vector<ElevationInfo>::iterator it;
		for( it = elevationData->begin(); it != elevationData->end(); ++it )
		{
			// Calculate longitude to meter ratio
			double longitudeMeterRatio =  (*it).location.Offset( GeoCoord( 0.0, 1.0, COORDSYS_METERS ) ).longitude -
			                              (*it).location.longitude;

			// TODO: Longitude to meter ratio should be modified to be the ratio between longitude degrees and X-axis.
			// The reference ratio should be used to calculate it.

			double x = ((*it).location.longitude - referencePoint.longitude) / longitudeMeterRatio;
			double y = (*it).elevation;
			double z = ((*it).location.latitude - referencePoint.latitude) / latitudeMeterRatio;
			terrainObject->position( x,	y, z );

			// Set some nice color
			terrainObject->colour( 0.0, (y/120.0), 1.0/(y/120.0) );
		}

		// Create the triangles
		for( unsigned int y=0; y < lod; ++y )
		{
			index = y * (lod+1);

			for( unsigned int x=0; x < lod; ++x )
			{
				// Now we got the index of the top left corner of current cell.
				// We'll use it to calculate the other indexes for the vertices
				// of triangles in the cell
				terrainObject->triangle( index, index+1, index+lod+1 );
				terrainObject->triangle( index+2+lod, index+1+lod, index+1 );
				index += 1;
			}
		}
	}
	terrainObject->end();

	terrainObject->convertToMesh( Ogre::String( "Mesh" + areaName ) );

	// Free resources
	elevationData->clear();
	free( elevationData );
}



std::vector<ElevationInfo>* TerrainLoader::LoadElevationData( const std::vector<GeoCoord> &locations )
{
	// Clear possible old data
	curlBuffer.clear();

	// Vector to hold elevation data.
	std::vector<ElevationInfo> *elevationData = new std::vector<ElevationInfo>(); 


	CURLcode res;


	// Create string, that contains the list of coordinates
	std::string locationListString;
	std::stringstream locationListStream;
	std::vector<GeoCoord>::const_iterator it;;

	for( it = locations.begin(); it != locations.end(); ++it )
	{
		locationListStream << (*it).latitude << "," << (*it).longitude;

		// If this isn't the last position append |
		if( it + 1 != locations.end() )
		{
			locationListStream << '|';
		}
	}
	locationListStream >> locationListString;


	// Set up CURL options and fetch the data
	curl_easy_setopt( app.curl, CURLOPT_URL, (baseURL + locationListString).c_str() );
	curl_easy_setopt( app.curl, CURLOPT_FOLLOWLOCATION, 0L );
	curl_easy_setopt( app.curl, CURLOPT_WRITEFUNCTION, &data_write );

	res = curl_easy_perform( app.curl );
	if( res != CURLE_OK )
	{
		throw std::exception( curl_easy_strerror( res ) );
	}


	// Copy the received data, this is because vjson modifies it.
	size_t bufferSize = sizeof( char )* curlBuffer.size();
	char *jsonBuffer = (char*)malloc( bufferSize + 1 );
	if( !jsonBuffer )
	{
		throw std::exception( "Reserving memory for json buffer failed!" );
	}

	strncpy( jsonBuffer, curlBuffer.c_str(), bufferSize );
	jsonBuffer[bufferSize] = 0;



	// Start parsing json data
	char *errorPos = 0;
	char *errorDesc = 0;
	int errorLine = 0;
	block_allocator allocator(1 << 10); // 1 KB per block
    
	json_value *root = json_parse( jsonBuffer, &errorPos, &errorDesc, &errorLine, &allocator );

	if( !root )
	{
		std::cout << "Tried to parse: \"" << curlBuffer << "\"\n";
		throw std::exception( ("JSON Exception: " + std::string( errorDesc )).c_str() );
	}


	// Check that the json data has data for the results
	json_value *results = nullptr;
	for( json_value *it = root->first_child; it; it = it->next_sibling )
	{
		if( strcmp( it->name, "results" ) != 0 )
		{
           continue;
		}

		results = it;
		break;
	}

	if( !results )
	{
		throw std::exception( "Failed to find results from json tree." );
	}
	
	// Start looping over the results
	for( json_value *it = results->first_child; it; it = it->next_sibling )
	{
		ElevationInfo point;

		// Loop over the result's fields
		for( json_value *child = it->first_child; child; child = child->next_sibling )
		{
			if( strcmp( child->name, "elevation" ) == 0 )
			{
				if( child->type == JSON_INT )
				{
					point.elevation = (long)child->int_value;
				}
				else if( child->type == JSON_FLOAT )
				{
					point.elevation = child->float_value;
				}
				else
				{
					throw std::exception( "Wrong type of data in the JSON" );
				}
			}
			else if( strcmp( child->name, "location" ) == 0 )
			{
				point.location.latitude = child->first_child->float_value;
				point.location.longitude = child->last_child->float_value;
				point.location.coordSystem = COORDSYS_DDDDDDDD;
			}
		}

		elevationData->push_back( point );
	}

	if( jsonBuffer )
	{
		free( jsonBuffer );
	}

	return elevationData;
}



void TerrainLoader::SetReferencePoint( const GeoCoord &reference )
{
	referencePoint = reference;
	referenceRatio = reference.Offset( GeoCoord( 0.0, 1.0, COORDSYS_METERS ) ).longitude -
	                 reference.longitude;
}
