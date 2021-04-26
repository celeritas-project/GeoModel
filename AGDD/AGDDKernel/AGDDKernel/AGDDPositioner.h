/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef AGDDPositioner_H
#define AGDDPositioner_H

#include "GeoModelKernel/GeoDefinitions.h"

#include <string>

class AGDDVolume;

class AGDDPositioner {
public:
	AGDDPositioner(std::string n,GeoTrf::Transform3D t);
	AGDDPositioner(std::string n,GeoTrf::Transform3D r,GeoTrf::Vector3D v);
	virtual ~AGDDPositioner() {;}
	std::string Volume();
	AGDDVolume *GetVolume();
	const GeoTrf::Transform3D& Transform();
    bool IsSensitiveDetector() {return m_isSensitiveDetector;}
	void SensitiveDetector(bool a) {m_isSensitiveDetector=a;}
	
private:

	bool m_isSensitiveDetector;
	std::string m_volume;
	AGDDVolume *m_theVolume;
	GeoTrf::Transform3D m_transform = GeoTrf::Transform3D::Identity();
};

#endif
