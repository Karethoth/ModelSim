#pragma once
#ifndef _TERRAINLOADER_HH_
#define _TERRAINLOADER_HH_

#include "predefs.hh"
#include "CoordinateSystem.hh"
#include "SimApp.hh"


typedef struct sElevationInfo
{
	GeoCoord location;
	double elevation;
} ElevationInfo;



class TerrainLoader
{
  public:
	TerrainLoader( SimApp& );
	void LoadArea( const GeoCoord &center, const double &degsLat, const double &degsLon, const unsigned char &lod, const std::string &areaName );
	void SetReferencePoint( const GeoCoord &reference );

  protected:
	  std::vector<ElevationInfo>* LoadElevationData( const std::vector<GeoCoord> &locations );

  private:
	SimApp &app;
	GeoCoord referencePoint;
	double referenceRatio;

	bool finished;
};

#endif
