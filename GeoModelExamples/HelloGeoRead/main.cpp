// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

/*
 * HelloGeoRead.cpp
 *
 *  Author:     Riccardo Maria BIANCHI @ CERN
 *  Created on: Nov, 2018
 *  Updated on: Jul, 2023
 *
 */

// GeoModel includes
#include "GeoModelDBManager/GMDBManager.h"
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelRead/ReadGeoModel.h"

// C++ includes
#include <cstdlib>  // EXIT_FAILURE
#include <fstream>
#include <iostream>

// Units
#include "GeoModelKernel/Units.h"
#define SYSTEM_OF_UNITS \
    GeoModelKernelUnits  // so we will get, e.g., 'GeoModelKernelUnits::cm'

GeoPhysVol* createTheWorld(GeoPhysVol* world) {
    if (world == nullptr) {
        //-----------------------------------------------------------------------------------//
// Define the materials that we shall use. //
// ----------------------------------------------------------------------------------//

// Define the units
#define gr SYSTEM_OF_UNITS::gram
#define mole SYSTEM_OF_UNITS::mole
#define cm3 SYSTEM_OF_UNITS::cm3

        // Define the chemical elements
        GeoElement* Nitrogen =
            new GeoElement("Nitrogen", "N", 7.0, 14.0067 * gr / mole);
        GeoElement* Oxygen =
            new GeoElement("Oxygen", "O", 8.0, 15.9995 * gr / mole);
        GeoElement* Argon =
            new GeoElement("Argon", "Ar", 18.0, 39.948 * gr / mole);
        GeoElement* Hydrogen =
            new GeoElement("Hydrogen", "H", 1.0, 1.00797 * gr / mole);

        // Define the materials
        double densityOfAir = 0.001214 * gr / cm3;
        GeoMaterial* air = new GeoMaterial("Air", densityOfAir);
        air->add(Nitrogen, 0.7494);
        air->add(Oxygen, 0.2369);
        air->add(Argon, 0.0129);
        air->add(Hydrogen, 0.0008);
        air->lock();

        const GeoBox* worldBox =
            new GeoBox(1000 * SYSTEM_OF_UNITS::cm, 1000 * SYSTEM_OF_UNITS::cm,
                       1000 * SYSTEM_OF_UNITS::cm);
        const GeoLogVol* worldLog = new GeoLogVol("WorldLog", worldBox, air);
        world = new GeoPhysVol(worldLog);
    }
    return world;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "\nERROR!\nusage: %s input.db\n\n", argv[0]);
        return 1;
    }
    // Get the input SQLite '.db' file containing the geometry
    std::string line;
    std::string fileName;
    fileName = argv[1];
    std::cout << "Using this SQLite '.db' file:" << fileName << std::endl;

    // check if DB file exists. If not, return.
    // FIXME: TODO: this check should go in the 'GMDBManager' constructor.
    std::ifstream infile(fileName.c_str());
    if (!infile.good()) {
        std::cout << "\n\tERROR!! A '" << fileName
                  << "' file does not exist!! Please, check the path of the "
                     "input file before running this program. Exiting...";
        exit(EXIT_FAILURE);
    }
    infile.close();

    // GET GEOMETRY FROM LOCAL DB

    // open the DB
    GMDBManager* db = new GMDBManager(fileName);
    /* Open database */
    if (db->checkIsDBOpen()) {
        std::cout << "OK! Database is open!\n";
    } else {
        std::cout << "Database is not open!\n";
        // return;
        throw;
    }

    // -- testing the input database
    //  std::cout << "Printing the list of all GeoMaterial nodes" << std::endl;
    //  db->printAllMaterials();
    //  std::cout << "Printing the list of all GeoElement nodes" << std::endl;
    //  db->printAllElements();

    /* setup the GeoModel reader */
    GeoModelIO::ReadGeoModel geoReader = GeoModelIO::ReadGeoModel(db);
    std::cout << "OK! ReadGeoModel is set." << std::endl;

    /* build the GeoModel geometry */
    GeoPhysVol* dbPhys =
        geoReader.buildGeoModel();  // builds the whole GeoModel tree in memory
    std::cout << "ReadGeoModel::buildGeoModel() done." << std::endl;

    std::cout << "Reading records from the imported geometry DB file..."
              << std::endl;
    geoReader.printDBTable("SerialIdentifiers");
    geoReader.printDBTable("IdentifierTags");

    // create the world volume container and
    // get the 'world' volume, i.e. the root volume of the GeoModel tree
    std::cout << "Getting the 'world' GeoPhysVol, i.e. the root volume of the "
                 "GeoModel tree"
              << std::endl;
    GeoPhysVol* world = createTheWorld(dbPhys);
    std::cout << "Getting the GeoLogVol used by the 'world' volume"
              << std::endl;
    const GeoLogVol* logVol = world->getLogVol();
    std::cout << "'world' GeoLogVol name: " << logVol->getName() << std::endl;
    std::cout << "'world' GeoMaterial name: "
              << logVol->getMaterial()->getName() << std::endl;

    // --- testing the imported Geometry

    // get number of children volumes
    unsigned int nChil = world->getNChildVols();
    std::cout << "'world' number of children: " << nChil << std::endl;

    // loop over all children nodes
    std::cout << "Looping over all 'volume' children (i.e., GeoPhysVol and "
                 "GeoFullPhysVol)..."
              << std::endl;
    for (unsigned int idx = 0; idx < nChil; ++idx) {
        PVConstLink nodeLink = world->getChildVol(idx);

        if (dynamic_cast<const GeoVPhysVol*>(&(*(nodeLink)))) {
            std::cout << "\t"
                      << "the child n. " << idx << " ";
            const GeoVPhysVol* childVolV = &(*(nodeLink));

            if (dynamic_cast<const GeoPhysVol*>(childVolV)) {
                const GeoPhysVol* childVol =
                    dynamic_cast<const GeoPhysVol*>(childVolV);
                std::cout << "is a GeoPhysVol, whose GeoLogVol name is: "
                          << childVol->getLogVol()->getName();
                std::cout << " and it has  " << childVol->getNChildVols()
                          << " child volumes" << std::endl;
            } else if (dynamic_cast<const GeoFullPhysVol*>(childVolV)) {
                const GeoFullPhysVol* childVol =
                    dynamic_cast<const GeoFullPhysVol*>(childVolV);
                std::cout << "is a GeoFullPhysVol, whose GeoLogVol name is: "
                          << childVol->getLogVol()->getName();
                std::cout << " and it has  " << childVol->getNChildVols()
                          << " child volumes" << std::endl;
            }
        } else if (dynamic_cast<const GeoNameTag*>(&(*(nodeLink)))) {
            std::cout << "\t"
                      << "the child n. " << idx << " is a GeoNameTag"
                      << std::endl;
            const GeoNameTag* childVol =
                dynamic_cast<const GeoNameTag*>(&(*(nodeLink)));
            std::cout << "\t\t GeoNameTag's name: " << childVol->getName()
                      << std::endl;
        } else if (dynamic_cast<const GeoMaterial*>(&(*(nodeLink)))) {
            std::cout << "\t"
                      << "the child n. " << idx << " is a GeoMaterial"
                      << std::endl;
            const GeoMaterial* childVol =
                dynamic_cast<const GeoMaterial*>(&(*(nodeLink)));
            std::cout << "\t\t GeoMaterial's name: " << childVol->getName()
                      << std::endl;
            std::cout << "\t\t GeoMaterial's number of elements: "
                      << childVol->getNumElements() << std::endl;
        }
    }

    std::cout << "Everything done." << std::endl;

    // return app.exec();
    return 0;
}
