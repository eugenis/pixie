//////////////////////////////////////////////////////////////////////////
//
// Simple class for raw regions data storage
//
//////////////////////////////////////////////////////////////////////////
#pragma once
#include <QList>
#include <vector>

class CRegions
{
public:
	explicit CRegions( const int hW, const int hH, const int vW, const int vH );	// Do provide dimensions, or else there's no sense
	QList< unsigned char * > regionHTop;
	QList< unsigned char * > regionHBottom;
	QList< unsigned char * > regionVLeft;
	QList< unsigned char * > regionVRight;

	unsigned int getHSize() const;	// Size (int bytes) of horizontal segment
	unsigned int getVSize() const;	// Size (int bytes) of vertical segment

//private:
	unsigned short hWidth;		// Width (in pixels) of horizontal segment
	unsigned short hHeight;		// Height (in pixels) of horizontal segment

	unsigned short vWidth;		// Width (in pixels) of vertical segment
	unsigned short vHeight;		// Height (in pixels) of vertical segment
};


struct SparseRegion {
  unsigned char *data;
  int width, height, stride;

SparseRegion(unsigned char* data, int width, int height, int stride) : data(data), width(width), height(height), stride(stride) {}
SparseRegion() : data(0), width(0), height(0), stride(0) {}
};

class SparseRegions {
public:
	explicit SparseRegions( const int hW, const int hH, const int vW, const int vH );	// Do provide dimensions, or else there's no sense
	std::vector<SparseRegion> regionHTop;
	std::vector<SparseRegion> regionHBottom;
	std::vector<SparseRegion> regionVLeft;
	std::vector<SparseRegion> regionVRight;

	unsigned int getHSize() const;	// Size (int bytes) of horizontal segment
	unsigned int getVSize() const;	// Size (int bytes) of vertical segment

//private:
	unsigned int hWidth;		// Width (in pixels) of horizontal segment
	unsigned int hHeight;		// Height (in pixels) of horizontal segment

	unsigned int vWidth;		// Width (in pixels) of vertical segment
	unsigned int vHeight;		// Height (in pixels) of vertical segment
};
