/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/
#include <fstream>

#include "GeoModelKernel/GeoVGeometryPlugin.h"

#include "GeoModelXMLParser/XercesParser.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/Units.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GDMLInterface/GDMLController.h"
#define SYSTEM_OF_UNITS GeoModelKernelUnits // so we will get, e.g., 'GeoModelKernelUnits::cm'

#include <iostream>

inline bool exists (const std::string& name) {
    std::ifstream f(name.c_str());
    return f.good();
}

class GDMLtoGM : public GeoVGeometryPlugin  {

 public:

  // Constructor:  
  GDMLtoGM();

  // Destructor:
  ~GDMLtoGM();

  // Creation of geometry:
  virtual void create(GeoPhysVol *world, GeoPublisher* publisher);

 private:

  // Illegal operations:
  //const GDMLtoGM & operator=(const GDMLtoGM &right)=delete;
  //GDMLtoGM(const GDMLtoGM &right) = delete;

};





GDMLtoGM::GDMLtoGM()
{
}


GDMLtoGM::~GDMLtoGM()
{
}


//## Other Operations (implementation)
void GDMLtoGM::create(GeoPhysVol *world, GeoPublisher*)
{
	char* fPath=getenv("GDML_FILE_NAME");
	std::string fileName;
	if (fPath!=NULL) fileName=std::string(fPath);
	else fileName="gdmlfile.xml";
	if (!exists(fileName)) {
		std::cout <<"GDMLtoGeo: input file "<<fileName<<" does not exist. quitting and returning nicely! "<<std::endl;
		return;
	}
	
  	std::cout<< "GDMLtoGeo: GDML to GeoModel Interface. Parsing gdml file "<<fileName<<" and setting world volume"<<std::endl;
	GDMLController controller("GDMLController");
	XercesParser xercesParser;
  	xercesParser.ParseFileAndNavigate(fileName);
	//std::cout << "done parsing "<<std::endl;
	GeoPhysVol* w=controller.getWorld();
	const std::string nameTag=stripPointer(w->getLogVol()->getName());
	//std::cout<< "GDMLtoGeo: setting top volume name to: "<<nameTag<<std::endl;
	GeoNameTag* gdmlTag=new GeoNameTag(nameTag);
	world->add(gdmlTag);
	world->add(w);
}

extern "C" GDMLtoGM *createGDMLtoGM() {
  return new GDMLtoGM();
}
