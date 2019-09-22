/*
 * HelloGeo.cpp
 *
 *  Created on: Apr, 2019
 *      Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch>
 */

// GeoShape nodes
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoTorus.h"
#include "GeoModelKernel/GeoTessellatedSolid.h"

// Geo core classes
#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoTransform.h"

// Geo I/O
#include "GeoModelDBManager/GMDBManager.h"
#include "GeoModelWrite/WriteGeoModel.h"

// Qt5
#include <QDebug>
#include <QFileInfo>

// C++
#include <iostream>


// Units
#include "GeoModelKernel/Units.h"
#define SYSTEM_OF_UNITS GeoModelKernelUnits // so we will get, e.g., 'GeoModelKernelUnits::cm'


int main(int argc, char *argv[])
{
	//-----------------------------------------------------------------------------------//
	// create the world volume container and
	// get the 'world' volume, i.e. the root volume of the GeoModel tree
	std::cout << "Creating the 'world' volume, i.e. the root volume of the GeoModel tree..." << std::endl;
	double densityOfAir = 0.1;
	const GeoMaterial* worldMat = new GeoMaterial("std::Air", densityOfAir);
	const GeoBox* worldBox = new GeoBox(1000*SYSTEM_OF_UNITS::cm, 1000*SYSTEM_OF_UNITS::cm, 1000*SYSTEM_OF_UNITS::cm);
	const GeoLogVol* worldLog = new GeoLogVol("WorldLog", worldBox, worldMat);
	GeoPhysVol* world = new GeoPhysVol(worldLog);


	//----------------------------------------------------------------------------------//
	// Get the materials
	// const GeoMaterial* matIron = theMaterialManager->getMaterial("std::Iron"); // Athena code
	// Bogus densities.  Later: read from database.
	double densityOfIron= 0.7;
	const GeoMaterial *matIron = new GeoMaterial("Iron Toy",densityOfIron);


	//-----------------------------------------------------------------------------------//
	// Create the shapes:                            //

	// A box
	GeoBox* box = new GeoBox(50*SYSTEM_OF_UNITS::cm, 50*SYSTEM_OF_UNITS::cm, 50*SYSTEM_OF_UNITS::cm);

	// A torus
	GeoTorus* torus = new GeoTorus(10*SYSTEM_OF_UNITS::cm /*Rmin*/,  20*SYSTEM_OF_UNITS::cm /*Rmax*/, 50*SYSTEM_OF_UNITS::cm /*Rtor*/,  0*SYSTEM_OF_UNITS::degree/*SPhi*/,  270*SYSTEM_OF_UNITS::degree/*DPhi*/);

	// A Tessellated Solid
	// -- A tessellated solid made of a single 2D quad surface
	GeoFacetVertex quadV1 = GeoFacetVertex(0.,0.,0.);
	GeoFacetVertex quadV2 = GeoFacetVertex(0., 50.*SYSTEM_OF_UNITS::cm, 0.);
	GeoFacetVertex quadV3 = GeoFacetVertex(50.*SYSTEM_OF_UNITS::cm, 50.*SYSTEM_OF_UNITS::cm, 0.);
	GeoFacetVertex quadV4 = GeoFacetVertex(50.*SYSTEM_OF_UNITS::cm,0.,0.);
	GeoQuadrangularFacet* quadFacet = new GeoQuadrangularFacet(quadV1, quadV2, quadV3, quadV4, GeoFacet::ABSOLUTE);
	GeoTessellatedSolid* tessQuad = new GeoTessellatedSolid();
	tessQuad->addFacet(quadFacet);

	// -- A tessellated solid made of a single 3D triangular surface
	GeoFacetVertex triV1 = GeoFacetVertex(0,0,50*SYSTEM_OF_UNITS::cm);
	GeoFacetVertex triV2 = GeoFacetVertex(0,50*SYSTEM_OF_UNITS::cm,0);
	GeoFacetVertex triV3 = GeoFacetVertex(50*SYSTEM_OF_UNITS::cm,0,0);
	GeoTriangularFacet* triFacet = new GeoTriangularFacet(triV1, triV2, triV3, GeoFacet::ABSOLUTE);
	GeoTessellatedSolid* tessTri = new GeoTessellatedSolid();
	tessTri->addFacet(triFacet);

	// -- A tessellated solid made of a 3D triangular facet and a 2D quad facet
	GeoTessellatedSolid* tessTriQuad = new GeoTessellatedSolid();
	tessTriQuad->addFacet(triFacet);
	tessTriQuad->addFacet(quadFacet);

	// -- A cube can be represented with 8 points, a triangulated_surface_set with pnmax= 24, 24 normals, and a list of 12 triangles.
	// TODO:


	//------------------------------------//
	// Define the space transforms to place the boxes:
	GeoTrf::Translate3D Tr1(-75*SYSTEM_OF_UNITS::cm, 0, 0); // cube
	GeoTrf::Translate3D Tr2( 75*SYSTEM_OF_UNITS::cm, 0, 0); // torus
	GeoTrf::Translate3D Tr3( -75*SYSTEM_OF_UNITS::cm, -150*SYSTEM_OF_UNITS::cm, 0); // tessellated solid quad
	GeoTrf::Translate3D Tr4( 75*SYSTEM_OF_UNITS::cm, -150*SYSTEM_OF_UNITS::cm, 0); // tessellated solid tri
	GeoTrf::Translate3D Tr5( -75*SYSTEM_OF_UNITS::cm, -225*SYSTEM_OF_UNITS::cm, 0); // tessellated solid tri

	GeoTransform* tr1 = new GeoTransform(Tr1);
	GeoTransform* tr2 = new GeoTransform(Tr2);
	GeoTransform* tr3 = new GeoTransform(Tr3);
	GeoTransform* tr4 = new GeoTransform(Tr4);
	GeoTransform* tr5 = new GeoTransform(Tr5);


	//------------------------------------//
	// Bundle the resulting compound object with a material into a logical volume, and create a physical volume with that:
	GeoLogVol* boxLog = new GeoLogVol("Box",box,matIron);
	GeoPhysVol* boxPhys = new GeoPhysVol(boxLog);

	GeoLogVol* torusLog = new GeoLogVol("Torus",torus,matIron);
	GeoPhysVol* torusPhys = new GeoPhysVol(torusLog);

	GeoLogVol* tess1Log = new GeoLogVol("Tessellated Quad",tessQuad,matIron);
	GeoPhysVol* tess1Phys = new GeoPhysVol(tess1Log);
	GeoLogVol* tess2Log = new GeoLogVol("Tessellated Tri",tessTri,matIron);
	GeoPhysVol* tess2Phys = new GeoPhysVol(tess2Log);
	GeoLogVol* tess3Log = new GeoLogVol("Tessellated TriQuad",tessTriQuad,matIron);
	GeoPhysVol* tess3Phys = new GeoPhysVol(tess3Log);



	//------------------------------------//
	// Now insert all of this into the world...                                           //
	world->add(tr1);
	world->add(boxPhys);
	world->add(tr2);
	world->add(torusPhys);
	world->add(tr3);
	world->add(tess1Phys);
	world->add(tr4);
	world->add(tess2Phys);
	world->add(tr5);
	world->add(tess3Phys);



	//------------------------------------//
	// Writing the geometry to file
	QString path = "geometry.db";

	// check if DB file exists. If not, return.
	// TODO: this check should go in the 'GMDBManager' constructor.
	if ( QFileInfo(path).exists() ) {
		qWarning() << "\n\tERROR!! A '" << path << "' file exists already!! Please, remove it before running this program.";
		qWarning() << "\tReturning..." << "\n";
		// return;
		exit(1);
	}

	// open the DB connection
	GMDBManager db(path);

	// check the DB connection
	if (db.isOpen())
		qDebug() << "OK! Database is open!";
	else {
		qDebug() << "Database ERROR!! Exiting...";
		return 1;
	}

	std::cout << "Dumping the GeoModel geometry to the DB file..." << std::endl;
	// Dump the tree volumes into a DB
	GeoModelIO::WriteGeoModel dumpGeoModelGraph(db); // init the GeoModel node action
	world->exec(&dumpGeoModelGraph); // visit all nodes in the GeoModel tree
	dumpGeoModelGraph.saveToDB(); // save to the local SQlite DB file
	std::cout << "DONE. Geometry saved." <<std::endl;

	std::cout << "\nTest - list of all the GeoMaterial nodes in the persistified geometry:" << std::endl;
	db.printAllMaterials();
	std::cout << "\nTest - list of all the GeoShape nodes in the persistified geometry:" << std::endl;
	db.printAllShapes();


	return 0;
}
