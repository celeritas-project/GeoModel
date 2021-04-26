/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

//  HECWheelConstruction.cxx 1.0.0
//  
//  This provides a mother volume for the ATLAS Front HEC or Rear HEC,
//  gets the modules from the HECModuleConstruction and arranges
//  them into a FrontWheel or RearWheel respectively.
//  The LAr gaps can be equipped with either one thick PAD+EST board
//  or with 3 individual boards (1 PAD + 2 EST). 
//  
//  M. Fincke-Keeler 
//  
//====================================================================
#include "LArGeoHec/HECWheelConstruction.h"
#include "LArGeoHec/HECClampConstruction.h"
#include "LArGeoHec/HECModuleConstruction.h"

#include "GeoModelKernel/GeoElement.h"  
#include "GeoModelKernel/GeoMaterial.h"  
#include "GeoModelKernel/GeoFullPhysVol.h"  
#include "GeoModelKernel/GeoPhysVol.h"  
#include "GeoModelKernel/GeoVPhysVol.h"  
#include "GeoModelKernel/GeoLogVol.h"  
#include "GeoModelKernel/GeoPcon.h"  
#include "GeoModelKernel/GeoTubs.h"  
#include "GeoModelKernel/GeoNameTag.h"  
#include "GeoModelKernel/GeoTransform.h"  
#include "GeoModelKernel/GeoAlignableTransform.h"  
#include "GeoModelKernel/GeoIdentifierTag.h"  
#include "GeoModelKernel/GeoSerialTransformer.h"
#include "GeoModelKernel/GeoSerialIdentifier.h"
#include "GeoModelKernel/GeoXF.h"
#include "GeoModelKernel/GeoDefinitions.h"
#include "GeoModelKernel/Units.h"
#define SYSTEM_OF_UNITS GeoModelKernelUnits

#include "GeoGenericFunctions/Variable.h"

#include "GeoXmlMatManager/GeoXmlMatManager.h"
#include "GeoXmlInpManager/GeoXmlInpManager.h"
#include "GeoXmlInpManager/GeoInpRecordset.h"

#include <string>
#include <cmath>
#include <iostream>


using SYSTEM_OF_UNITS::cm;
using SYSTEM_OF_UNITS::mm;
using SYSTEM_OF_UNITS::deg;
using GeoTrf::RotateZ3D;

//Constructor
LArGeo::HECWheelConstruction::HECWheelConstruction(bool fullGeo, std::string wheelType, bool threeBoards, bool posZSide)
  : m_physiHECWheel(0)
  , m_fullGeo(fullGeo)
{
  m_threeBoards = threeBoards;
  m_posZSide    = posZSide;
  m_wheelType   = wheelType;
  if (m_wheelType=="front") m_frontWheel  = true;
  else                      m_frontWheel  = false;
}

//~Destructor
LArGeo::HECWheelConstruction::~HECWheelConstruction() 
{;}


GeoFullPhysVol* LArGeo::HECWheelConstruction::GetEnvelope()
{
  if(m_physiHECWheel) return m_physiHECWheel->clone();

  GeoXmlInpManager* inpman = GeoXmlInpManager::getManager();
  const GeoXmlMatManager* materialManager = GeoXmlMatManager::getManager();  

  std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
  std::cout << "+                                                             +" << std::endl;
  std::cout << "+         Start of HECWheel GeoModel definition               +" << std::endl;
  std::cout << "+                                                             +" << std::endl;
  std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" << std::endl;
  
  const GeoMaterial *LAr  = materialManager->getMaterial("std::LiquidArgon");
  if (!LAr) throw std::runtime_error("Error in HECWheelConstruction, std::LiquidArgon is not found.");
  
  const GeoMaterial *Iron  = materialManager->getMaterial("std::Iron");
  if (!Iron) throw std::runtime_error("Error in HECWheelConstruction, std::Iron is not found.");
  
  GeoInpRecordset_ptr hecLongitudinalBlock = inpman->getRecordsetPtr("HecLongitudinalBlock");
  GeoInpRecordset_ptr hadronicEndcap       = inpman->getRecordsetPtr("HadronicEndcap");

  //----------------------------------------------------------------
  // Collect all the numbers we need from the database: 
  //----------------------------------------------------------------

  // This will have to come into action once we are all ready to go cold:
  // (what's still missing right now are the proper densities to go with it) 
  //double shrinkCold = (*coldContraction)[0]->getDouble("ABSORBERCONTRACTION");

  double shrinkCold    = 1.0;   // thermal expansion factor: 1.0 = warm geometry 
 
  int    moduleNumber          = (*hadronicEndcap)[0].getInt("NSCT");
  double drModOverlap          = shrinkCold * (*hadronicEndcap)[0].getDouble("DRMODOVERLAP")*cm;
  double rOuter                = shrinkCold * (*hecLongitudinalBlock)[0].getDouble("BLRMX")*cm;
  if (rOuter<2100.*mm) rOuter  = shrinkCold * 2130*mm;  // Needs fixing in database to make room for HEC Clamping bars! 
                                           // Caution: We'll need 2 Routers - One for Wheel, one for Module!
  double rInner1               = shrinkCold * (*hecLongitudinalBlock)[0].getDouble("BLRMN")*cm;
  double rInner2               = shrinkCold * (*hecLongitudinalBlock)[1].getDouble("BLRMN")*cm;
  double firstFrontAbsThickness= shrinkCold * (*hecLongitudinalBlock)[0].getDouble("PLATE0")*cm;
  double firstRearAbsThickness = shrinkCold * (*hecLongitudinalBlock)[3].getDouble("PLATE0")*cm;
  double frontAbsThickness     = shrinkCold * (*hadronicEndcap)[0].getDouble("PLATE_0")*cm;
  double rearAbsThickness      = shrinkCold * (*hadronicEndcap)[0].getDouble("PLATE_1")*cm;
  double gapSize               = shrinkCold * (*hadronicEndcap)[0].getDouble("LARG")*cm;
  int    gapNumber[7];
  for (int id=0; id<7;id++) gapNumber[id] = (int) (*hecLongitudinalBlock)[id].getDouble("BLMOD");

 
  //---- Calculate the sizes of the 7 longitudinal Depths -------------------------------------
  double depthSize[7];
  depthSize[0]     = firstFrontAbsThickness + gapNumber[0]*(frontAbsThickness+gapSize);
  depthSize[1]     = gapNumber[1]*(frontAbsThickness+gapSize);
  depthSize[2]     = gapNumber[2]*(frontAbsThickness+gapSize);
  depthSize[3]     = firstRearAbsThickness + gapNumber[3]*(rearAbsThickness+gapSize);
  depthSize[4]     = gapNumber[4]*(rearAbsThickness+gapSize);
  depthSize[5]     = gapNumber[5]*(rearAbsThickness+gapSize);
  depthSize[6]     = gapNumber[6]*(rearAbsThickness+gapSize);

  //--- From the depth Size, calculate the boundary planes of the Wheel Mother volume ---------
  //    (Add 0.2 mm to avoid clashes)
  double g4allow   = shrinkCold * 0.2*mm;
  double zCoordinate[4];
  if (m_frontWheel){
    zCoordinate[0] = shrinkCold * 0.0*cm; 
    zCoordinate[1] = depthSize[0]; //280.5*mm;  (end of the section with the long plates)
    zCoordinate[2] = depthSize[0] + g4allow; //280.6*mm; 
    zCoordinate[3] = depthSize[0] + depthSize[1] + depthSize[2] + g4allow; // End of front Module  
  }
  else{
    zCoordinate[0] = shrinkCold * 0.0*cm; 
    zCoordinate[1] = depthSize[3] + depthSize[4] + depthSize[5] + depthSize[6] + g4allow; // End of rear
  }

  //---- And here we have some (yikes!) hard-coded values: -------------------------------------
  // The somewhat odd looking phistart arranges that the y-axis lines up
  // with the (radial) symmetry axis of of the module. 
  // This is important for the calculator.
  //  
  double modulePhistart      = 264.375*deg; // (270.-11.25/2.)*deg
  double moduleDeltaPhi      = 2*M_PI/moduleNumber; //11.25*deg;  

  int  nZplane = 4;
  if (!m_frontWheel) nZplane = 2;



  //----------------------------------------------------------------
  //   HECWheel 
  //----------------------------------------------------------------
  GeoPcon* solidHECWheel = new GeoPcon(0., 2.*M_PI);
  for (int i=0; i< nZplane; i++) 
     {
        double innerRadius = rInner2;
        if (i<2 && m_frontWheel) innerRadius=rInner1;  
        solidHECWheel->addPlane(zCoordinate[i],innerRadius-drModOverlap,rOuter);
     }
  std::string hecFrontName = "LAr::HEC::LiquidArgon";
  const GeoLogVol* logicHECWheel = new GeoLogVol(hecFrontName, solidHECWheel , LAr);
  GeoFullPhysVol* physiHECWheel = new GeoFullPhysVol(logicHECWheel);


  if(m_fullGeo) {

    //----------------------------------------------------------------
    //   Get HEC Modules
    //----------------------------------------------------------------
    // NB:  HECModuleConstruction( threeBoards, frontWheel, TB,TByear) ;

    HECModuleConstruction HECModule(m_threeBoards,m_frontWheel,false,0) ;
    GeoVPhysVol* moduleEnvelope = HECModule.GetEnvelope();
    
    //----------------------------------------------------------------
    //   Place Modules into HEC Wheel
    //----------------------------------------------------------------
    // Rotation angle is: 11.25/2 + Index *  11.25*deg
    // Modules are numbered mirror-symmetric in pos/neg z-side!
    
    GeoSerialIdentifier  *sIF = new GeoSerialIdentifier(0);
    GeoGenfun::Variable    Index;
      
    if (m_posZSide) {
      GeoGenfun::GENFUNCTION ModuleRotationAngle = -modulePhistart + moduleDeltaPhi*Index;
      GeoXF::TRANSFUNCTION t    = GeoXF::Pow(RotateZ3D(1.0),ModuleRotationAngle);
      GeoSerialTransformer *sTF = new GeoSerialTransformer (moduleEnvelope,&t,moduleNumber);
      physiHECWheel->add(sIF);
      physiHECWheel->add(sTF);
    } else {
      //For the neg z-side have to build everything in the opposite sense from pos-z wheel.
      GeoGenfun::GENFUNCTION ModuleRotationAngle1 = -modulePhistart+180*deg-moduleDeltaPhi - moduleDeltaPhi*Index;
      GeoXF::TRANSFUNCTION t1    = GeoXF::Pow(RotateZ3D(1.0),ModuleRotationAngle1);
      GeoSerialTransformer *sTF1 = new GeoSerialTransformer (moduleEnvelope,&t1,moduleNumber);
      physiHECWheel->add(sIF);
      physiHECWheel->add(sTF1);
    }

    //----------------------------------------------------------------
    //   Add Outer Connecting Bars to HEC Wheel
    //----------------------------------------------------------------
    
    HECClampConstruction HECClamp (m_frontWheel,m_posZSide);
    HECClamp.AddClamps(physiHECWheel); 
  }
    
  return physiHECWheel;
}

