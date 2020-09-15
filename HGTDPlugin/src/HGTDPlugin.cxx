/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/


#include "GeoXmlMatManager/GeoXmlMatManager.h"
#include "GeoModelKernel/GeoVGeometryPlugin.h"
#include "GeoModelKernel/GeoDefinitions.h"
#include "GeoModelKernel/GeoMaterial.h"
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoTube.h"
#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoTorus.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoTransform.h"
#include "GeoModelKernel/GeoSerialDenominator.h"
#include "GeoModelKernel/GeoAlignableTransform.h"
#include "GeoModelKernel/GeoSerialTransformer.h"
#include "GeoModelKernel/GeoPublisher.h"  // to publish lists of nodes

#include "GeoGenericFunctions/AbsFunction.h"
#include "GeoGenericFunctions/Variable.h"
#include "GeoGenericFunctions/Sin.h"
#include "GeoGenericFunctions/Cos.h"
#include "GeoModelKernel/Units.h"
#include <map>
#define SYSTEM_OF_UNITS GeoModelKernelUnits // so we will get, e.g., 'GeoModelKernelUnits::cm'

using namespace GeoGenfun;
using namespace GeoXF;


class HGTDPlugin : public GeoVGeometryPlugin  {

 public:

  // Constructor:
  HGTDPlugin( std::string pluginName, std::unique_ptr<GeoPublisher> publisher ) : GeoVGeometryPlugin( pluginName, std::move(publisher) ) {};

  // Destructor:
  ~HGTDPlugin();

  // Creation of geometry:
  virtual void create(GeoPhysVol *world, GeoPublisher*) override;

 private:

  // Illegal operations:
  const HGTDPlugin & operator=(const HGTDPlugin &right)=delete;
  HGTDPlugin(const HGTDPlugin &right) = delete;

  
  struct GeoCylVolParams {
    std::string name;
    double rMin;
    double rMax;
    double zHalf;
    double zOffsetLocal;
    std::string material;
  };

  struct GeoBoxVolParams {
    std::string name;
    double xHalf;
    double yHalf;
    double zHalf;
    double zOffsetLocal;
    std::string material;
  };

  struct HgtdGeoParams {
    double rMid;
    double rOuter;
    double diskRotation;
    double rowSpaceSide;
    double rowBacksideInnerShift;
    double rowBacksideOuterShift;
    double moduleSpaceInner;
    double moduleSpaceOuter;
    double flexSheetSpacing;
  } m_hgtdPars;

  struct ModulePosition {
    double x;
    double y;
    double phiRotation;
    bool flipped;
    int row;
    int el_in_row;
  };
  
  GeoFullPhysVol* createEnvelope(bool bPos);
  void buildHgtdGeoTDR(const GeoXmlMatManager* matmanager, GeoFullPhysVol* HGTDparent, bool isPos);
  std::vector<ModulePosition> calculateHgtdModulePositionsInQuadrant(int layer);
  std::vector<ModulePosition> calculateHgtdModulePositionsInRow(int row, bool back);
  
  std::map<std::string,GeoCylVolParams> m_cylVolPars={
						      {"HGTD_mother",             {"HGTD_mother",         120,     1100,    62.5,    -3252,   "std::Air"}},
						      {"HGTD::FrontCover",        {"HGTD::FrontCover",    120,     1000,    7.5   ,  0   ,    "sct::CFiberSupport"}},
						      {"HGTD::FlexPackage",       {"HGTD::FlexPackage",   120,     660 ,    2     ,  0   ,    "std::Air"}},
						      {"HGTD::FlexTube",          {"HGTD::FlexTube",      120,     660 ,    .175  ,  0   ,    "sct::CuKapton"}},
						      {"HGTD::ModuleLayer0",      {"HGTD::ModuleLayer0",  120,     660 ,    3.75  ,  0   ,    "std::Air"}},
						      {"HGTD::ModuleLayer1",      {"HGTD::ModuleLayer1",  120,     660 ,    3.75  ,  0   ,    "std::Air"}},
						      {"HGTD::ModuleLayer2",      {"HGTD::ModuleLayer2",  120,     660 ,    3.75  ,  0   ,    "std::Air"}},       
						      {"HGTD::ModuleLayer3",      {"HGTD::ModuleLayer3",  120,     660 ,    3.75  ,  0   ,    "std::Air"}},       
						      {"HGTD::CoolingPlate",      {"HGTD::CoolingPlate",  120,     920 ,    3     ,  0   ,    "sct::CFiberSupport"}},
						      {"HGTD::SupportPlate",      {"HGTD::SupportPlate",  120,     660 ,    .5    ,  0   ,    "std::Aluminium"}},
						      {"HGTD::ToleranceFront",    {"HGTD::ToleranceFront",120,     660 ,    1     ,  0   ,    "std::Air"}},
						      {"HGTD::ToleranceBack",     {"HGTD::ToleranceBack", 120,     660 ,    1     ,  0   ,    "std::Air"}},
						      {"HGTD::ToleranceMid",      {"HGTD::ToleranceMid",  120,     660 ,    1     ,  0   ,    "std::Air"}},
						      {"HGTD::ModeratorIn",       {"HGTD::ModeratorIn",   120,     900 ,    15    ,  0   ,    "LAr::BoratedPolyethelyne"}},
						      {"HGTD::ModeratorOut",      {"HGTD::ModeratorOut",  120,     1100,    10    ,  0   ,    "LAr::BoratedPolyethelyne"}},
						      {"HGTD::BackCover",         {"HGTD::BackCover",     120,     1100,    4     ,  0   ,    "sct::CFiberSupport"}},
						      {"HGTD::PeriphElec",        {"HGTD::PeriphElec",    674,     900 ,    1     ,  2   ,    "LAr::FEBoards"}}
  };

  std::map<std::string,GeoBoxVolParams> m_boxVolPars={
						      {"HGTDModule0"       ,  {"HGTDModule0"    ,       11   ,   20   ,   1.75 ,   0  ,     "std::Air"}},
						      {"HGTDModule1"       ,  {"HGTDModule1"    ,       11   ,   20   ,   1.75 ,   0  ,     "std::Air"}},
						      {"HGTDModule2"       ,  {"HGTDModule2"    ,       11   ,   20   ,   1.75 ,   0  ,     "std::Air"}},
						      {"HGTDModule3"       ,  {"HGTDModule3"    ,       11   ,   20   ,   1.75 ,   0  ,     "std::Air"}},
						      {"HGTD::Hybrid"      ,  {"HGTD::Hybrid"   ,       10.25,   20   ,   .175 ,   0  ,     "sct::CuKapton"}},
						      {"HGTD::GlueSensor"  ,  {"HGTD::GlueSensor",      10.25,   20   ,   .04  ,   0  ,     "sct::Epoxy"}},
						      {"HGTD::GlueAsic"    ,  {"HGTD::GlueAsic" ,       11   ,   20   ,   .04  ,   0  ,     "sct::Epoxy"}},
						      {"HGTDSiSensor0"     ,  {"HGTDSiSensor0"  ,       10.25,   20   ,   .025 ,   0  ,     "std::Silicon"}},
						      {"HGTDSiSensor1"     ,  {"HGTDSiSensor1"  ,       10.25,   20   ,   .025 ,   0  ,     "std::Silicon"}},
						      {"HGTDSiSensor2"     ,  {"HGTDSiSensor2"  ,       10.25,   20   ,   .025 ,   0  ,     "std::Silicon"}},
						      {"HGTDSiSensor3"     ,  {"HGTDSiSensor3"  ,       10.25,   20   ,   .025 ,   0  ,     "std::Silicon"}},
						      {"HGTD::LGADInactive",  {"HGTD::LGADInactive",    10.25,   20   ,   .1   ,   0  ,     "std::Silicon"}},
						      {"HGTD::ASIC"        ,  {"HGTD::ASIC"     ,       11   ,   20   ,   .15  ,   0  ,     "std::Silicon"}}
  };


};



HGTDPlugin::~HGTDPlugin()
{
}


//## Other Operations (implementation)
void HGTDPlugin::create(GeoPhysVol *world, GeoPublisher* publisher)
{


  GeoFullPhysVol* HGTD_EnvelopePos = createEnvelope(true);
  GeoFullPhysVol* HGTD_EnvelopeNeg = createEnvelope(false);
  
  world->add(new GeoNameTag("HGTD"));
  
   
  world->add( new GeoNameTag("HGTD_Pos"));
  // world->add(xfHGTDPos); // StoredAlignX
  world->add( new GeoTransform(GeoTrf::TranslateZ3D(3500)));
  world->add(HGTD_EnvelopePos);
  world->add( new GeoNameTag("HGTD_Neg"));
  // world->add(xfHGTDNeg); // StoredAlignX
  world->add( new GeoTransform(GeoTrf::RotateY3D(180.0*SYSTEM_OF_UNITS::deg)));
  world->add( new GeoTransform(GeoTrf::TranslateZ3D(3500)));
  world->add(HGTD_EnvelopeNeg);
  

  
}

extern "C" HGTDPlugin *createHGTDPlugin() {
    auto publisher = std::make_unique<GeoPublisher>(); 
    return new HGTDPlugin( "HGTDPlugin", std::move(publisher) );
}

GeoFullPhysVol* HGTDPlugin::createEnvelope(bool bPos) {

    // the envelope to be returned
    GeoFullPhysVol *world_physical_hgtd = NULL;

    // Get the materials from the material manager:-----------------------------------------------------//
    //

    const GeoXmlMatManager* materialManager = GeoXmlMatManager::getManager();


    // These geometry flags were taken from the tag name in RDBAccessSvc.
    bool hgtdTiming    = false;
    bool hgtdPreshower = false; 
    bool hgtdTDR       = true;
    bool hgtdTDR2      = false;

    // detailed geometry to be used for TDR samples
    if ( hgtdTDR ) {

   
        // these aren't in the db yet or are incorrect as of now, need to be added by hand for now
        m_cylVolPars["HGTD::PeriphElec"].zHalf = 1.;
        m_cylVolPars["HGTD::PeriphElec"].zOffsetLocal = 2.;//space wrt cooling layer
        m_cylVolPars["HGTD_mother"].rMin= 100;
        m_cylVolPars["HGTD::ToleranceBack"].rMax= 900; // It's a little awkward to have an outer R for a tolerance. Do we need to change this? /CO
        //DZ_rmin = (125-20)/2; zpos = 62.5-20-DZ/2=-10
        m_cylVolPars["HGTD::InnerRCover"]  = {"HGTD::InnerRCover", 115., 120., 105./2, -10., "sct::CFiberSupport"}; // not yet in db!
        //Dz_rmax = (125-20-8-15)/2; zpos = 62.6-20-8-dz/2
        m_cylVolPars["HGTD::OuterRCover"]  = {"HGTD::OuterRCover", 995., 1000., 82./2, -6.5, "sct::CFiberSupport"}; // not yet in db!
        m_cylVolPars["HGTD::CoolingTube"] = {"HGTD::CoolingTubes", 0, 0, 2.0, 0, "std::SSteel"};
        m_cylVolPars["HGTD::CoolingTubeFluid"] = {"HGTD::CoolingTubeFluid", 0, 0, 1.5, 0, "pix::CO2_Liquid"};

        // hack after fix of ASIC thickness (and material) in HGTD-TDR-01 tag (ATLAS-P2-ITK-17-04-02 and later)
        double moduleSpaceHalfZ = 0.0;
        if (hgtdTDR2) moduleSpaceHalfZ = 0.225;
        m_boxVolPars["HGTD::ModuleSpace"] = {"HGTD::ModuleSpace", 11, 20, moduleSpaceHalfZ, 0, "std::Air"};

        // These parameters are not in the db (yet) and don't fit into the cylinder or box structures used above
        m_hgtdPars = { 320., // rMid
                       640., // rOuter - only used in one place, and there 20 mm is added to it...
                       15.,  // diskRotation (in degrees)
                       1.,   // rowSpaceSide
                       4.,   // rowBacksideInnerShift
                       17.,  // rowBacksideOuterShift
                       1.5,  // moduleSpaceInner
                       12.5, // moduleSpaceOuter
                       0.456 // flexSheetSpacing
                     };

        GeoTube        *world_solid_hgtd    = new GeoTube(m_cylVolPars["HGTD_mother"].rMin, m_cylVolPars["HGTD_mother"].rMax, m_cylVolPars["HGTD_mother"].zHalf);
        GeoLogVol      *world_logical_hgtd  = new GeoLogVol("HGTD_mother", world_solid_hgtd, materialManager->getMaterial(m_cylVolPars["HGTD_mother"].material));
        world_physical_hgtd = new GeoFullPhysVol(world_logical_hgtd);

        //now build HGTD
	buildHgtdGeoTDR(materialManager, world_physical_hgtd, bPos);
    }
#ifdef BUILD_OLDER
    // older geometries used for EoI and TP samples
    else if ( hgtdTiming || hgtdPreshower ) {
        IRDBRecordset_ptr hgtdTubs   = m_athenaComps->rdbAccessSvc()->getRecordsetPtr("HGTDTubs", detectorKey, detectorNode);
        if (m_fullGeo) {
            double hgtdToleranceSmall = 0.0;
            // get the info from the DB
            IRDBRecordset::const_iterator first = hgtdTubs->begin();
            IRDBRecordset::const_iterator last = hgtdTubs->end();
            IRDBRecordset::const_iterator it;
            // fill the names and the info
            std::vector <std::string> hgtdName;
            std::vector <double> hgtdRmin;
            std::vector <double> hgtdRmax;
            std::vector <double> hgtdDeltaZ;
            std::vector <double> hgtdDeltaY;
            std::vector <std::string> hgtdMaterial;

            // special treatment for mother since we need its position
            double zposMM = 0.;

            // get indices for the various db entries and fill vectors with the properties in each
            int iMother            = -1;
            int iAbsorber          = -1;
            int iInsulation        = -1;
            int iCooling           = -1;
            int iSensor            = -1;
            int iGlue              = -1;
            int iPCB               = -1;
            int iElectronics       = -1;
            int iHVKapton          = -1;
            int iMecTolElectronics = -1;
            int imecTolCooling     = -1;
            int iColdTubeCoat      = -1;
            int iColdTube          = -1;
            int iSupportStruct     = -1;

            for (it=first; it!=last; it++) {
                hgtdName.push_back((*it)->getString("TUBE"));
                hgtdRmin.push_back((*it)->getDouble("RMIN"));
                hgtdRmax.push_back((*it)->getDouble("RMAX"));
                hgtdDeltaZ.push_back((*it)->getDouble("DZ"));
                hgtdDeltaY.push_back((*it)->getDouble("DELTAY"));
                hgtdMaterial.push_back((*it)->getString("MATERIAL"));

                std::string theName = hgtdName[hgtdName.size()-1];
                if ( theName == "HGTD_mother" ) {
                    zposMM = (*it)->getDouble("ZPOS");
                    iMother = it - first;
                }
                else if ( theName == "Insulation" ) iInsulation = it - first;
                else if ( theName == "Cooling" ) iCooling = it - first;
                else if ( theName == "Sensor" ) iSensor = it - first;
                else if ( theName == "Glue" ) iGlue = it - first;
                else if ( theName == "PCB" ) iPCB = it - first;
                else if ( theName == "Electronics" ) iElectronics = it - first;
                else if ( theName == "HVKapton" ) iHVKapton = it - first;
                else if ( theName == "MecTolElectronics" ) iMecTolElectronics = it - first;
                else if ( theName == "mecTolCooling" ) imecTolCooling = it - first;
                else if ( theName == "ColdTubeCoat" ) iColdTubeCoat = it - first;
                else if ( theName == "ColdTube" ) iColdTube = it - first;
                else if ( theName == "SupportStruct" ) iSupportStruct = it - first;
                else if ( theName == "Absorber" ) iAbsorber = it - first;
            }
            // check if one of the needed entries were not found
            if (iMother       == -1 || iInsulation  == -1 || iCooling       == -1 || iSensor            == -1 || iGlue          == -1 ||
                iPCB          == -1 || iElectronics == -1 || iHVKapton      == -1 || iMecTolElectronics == -1 || imecTolCooling == -1 ||
                iColdTubeCoat == -1 || iColdTube    == -1 || iSupportStruct == -1)
                throw std::runtime_error("Error in HGTD_DetectorFactory: unable to get all HGTD parameters from the database!");
            // only require that the absorber entry was found if the hgtdPreshower geo was requested
            if ( hgtdPreshower && iAbsorber == -1 )
                throw std::runtime_error("Error in HGTD_DetectorFactory: unable to get HGTD Absorber parameters from the database for the requested preshower!");

            // build the HGTD Mother volume
            GeoTube     *world_solid_hgtd    = new GeoTube(hgtdRmin[iMother],hgtdRmax[iMother],hgtdDeltaZ[iMother]);
            GeoMaterial *material_world_hgtd = materialManager->getMaterial(hgtdMaterial[iMother]);
            GeoLogVol   *world_logical_hgtd  = new GeoLogVol(hgtdName[iMother],world_solid_hgtd,material_world_hgtd);
            world_physical_hgtd = new GeoFullPhysVol(world_logical_hgtd);

            // construct the detectors from the back to the front
            std::vector<std::string> hgtdVolName;
            std::vector<unsigned int> hgtdVolMap;
            hgtdVolName.push_back("SupportStructHgtd1");     hgtdVolMap.push_back(iSupportStruct);
            hgtdVolName.push_back("CoolingHgtd4");           hgtdVolMap.push_back(iCooling);
            hgtdVolName.push_back("mecTolElectronicsHgtd3"); hgtdVolMap.push_back(iMecTolElectronics);
            hgtdVolName.push_back("ElectronicsHgtd3");       hgtdVolMap.push_back(iElectronics);
            hgtdVolName.push_back("PCBHgtd3");               hgtdVolMap.push_back(iPCB);
            hgtdVolName.push_back("GlueHgtd3");              hgtdVolMap.push_back(iGlue);
            hgtdVolName.push_back("HGTDSiSensor3");          hgtdVolMap.push_back(iSensor);
            hgtdVolName.push_back("HVKaptonHgtd3");          hgtdVolMap.push_back(iHVKapton);
            hgtdVolName.push_back("mecTolCoolingHgtd3");     hgtdVolMap.push_back(imecTolCooling);
            hgtdVolName.push_back("CoolingHgtd3");           hgtdVolMap.push_back(iCooling);
            if ( hgtdPreshower ) {
                hgtdVolName.push_back("AbsorberHgtd2");          hgtdVolMap.push_back(iAbsorber);
            }
            hgtdVolName.push_back("mecTolElectronicsHgtd2"); hgtdVolMap.push_back(iMecTolElectronics);
            hgtdVolName.push_back("ElectronicsHgtd2");       hgtdVolMap.push_back(iElectronics);
            hgtdVolName.push_back("PCBHgtd2");               hgtdVolMap.push_back(iPCB);
            hgtdVolName.push_back("GlueHgtd2");              hgtdVolMap.push_back(iGlue);
            hgtdVolName.push_back("HGTDSiSensor2");          hgtdVolMap.push_back(iSensor);
            hgtdVolName.push_back("HVKaptonHgtd2");          hgtdVolMap.push_back(iHVKapton);
            hgtdVolName.push_back("mecTolCoolingHgtd2");     hgtdVolMap.push_back(imecTolCooling);
            hgtdVolName.push_back("CoolingHgtd2");           hgtdVolMap.push_back(iCooling);
            if ( hgtdPreshower ) {
                hgtdVolName.push_back("AbsorberHgtd1");          hgtdVolMap.push_back(iAbsorber);
            }
            hgtdVolName.push_back("mecTolElectronicsHgtd1"); hgtdVolMap.push_back(iMecTolElectronics);
            hgtdVolName.push_back("ElectronicsHgtd1");       hgtdVolMap.push_back(iElectronics);
            hgtdVolName.push_back("PCBHgtd1");               hgtdVolMap.push_back(iPCB);
            hgtdVolName.push_back("GlueHgtd1");              hgtdVolMap.push_back(iGlue);
            hgtdVolName.push_back("HGTDSiSensor1");          hgtdVolMap.push_back(iSensor);
            hgtdVolName.push_back("HVKaptonHgtd1");          hgtdVolMap.push_back(iHVKapton);
            hgtdVolName.push_back("mecTolCoolingHgtd1");     hgtdVolMap.push_back(imecTolCooling);
            hgtdVolName.push_back("CoolingHgtd1");           hgtdVolMap.push_back(iCooling);
            if ( hgtdPreshower ) {
                hgtdVolName.push_back("AbsorberHgtd0");          hgtdVolMap.push_back(iAbsorber);
            }
            hgtdVolName.push_back("mecTolElectronicsHgtd0"); hgtdVolMap.push_back(iMecTolElectronics);
            hgtdVolName.push_back("ElectronicsHgtd0");       hgtdVolMap.push_back(iElectronics);
            hgtdVolName.push_back("PCBHgtd0");               hgtdVolMap.push_back(iPCB);
            hgtdVolName.push_back("GlueHgtd0");              hgtdVolMap.push_back(iGlue);
            hgtdVolName.push_back("HGTDSiSensor0");          hgtdVolMap.push_back(iSensor);
            hgtdVolName.push_back("HVKaptonHgtd0");          hgtdVolMap.push_back(iHVKapton);
            hgtdVolName.push_back("mecTolCoolingHgtd0");     hgtdVolMap.push_back(imecTolCooling);
            hgtdVolName.push_back("CoolingHgtd0");           hgtdVolMap.push_back(iCooling);
            hgtdVolName.push_back("SupportStructHgtd0");     hgtdVolMap.push_back(iSupportStruct);
            hgtdVolName.push_back("InsulationHgtd0");        hgtdVolMap.push_back(iInsulation);

            // build the volumes in the requested order
            std::vector<GeoMaterial *> hgtdGeoMaterial;
            std::vector<double>        hgtdLocalZPos;
            std::vector<GeoTube *>     hgtdSolidVol;
            std::vector<GeoLogVol *>   hgtdLogVol;
            std::vector<GeoPhysVol *>  hgtdPhysVol;
            unsigned int index, indexCoolingVolume;
            for (unsigned int ii = 0;ii<hgtdVolName.size();ii++) {
                index     = hgtdVolMap[ii];
                hgtdGeoMaterial.push_back( materialManager->getMaterial(hgtdMaterial[index]) );
                // the z coordinate needs special care
                if (ii== 0 )
                    hgtdLocalZPos.push_back(hgtdDeltaZ[iMother] - hgtdDeltaZ[index] - hgtdToleranceSmall);
                else
                    hgtdLocalZPos.push_back(hgtdLocalZPos[ii-1] - hgtdDeltaZ[ hgtdVolMap[ii-1] ] - hgtdDeltaZ[index] - hgtdToleranceSmall);
                hgtdSolidVol.push_back( new GeoTube(hgtdRmin[index],hgtdRmax[index],hgtdDeltaZ[index]) );
                hgtdLogVol.push_back( new GeoLogVol(hgtdVolName[ii],hgtdSolidVol[ii],hgtdGeoMaterial[ii]) );
                hgtdPhysVol.push_back( new GeoPhysVol(hgtdLogVol[ii]) );
                world_physical_hgtd->add(new GeoTransform(HepGeom::TranslateZ3D(hgtdLocalZPos[ii])));
                world_physical_hgtd->add(hgtdPhysVol[ii]);

                // debug print-outs
                msg(MSG::DEBUG) << "HGTD built volume " << std::setw(24) << hgtdMaterial[index]
                                << " at position " << std::setw(8) <<  hgtdLocalZPos[ii]
                                << " with Rmin " << std::setw(8) <<  hgtdRmin[index]
                                << " Rmax " << std::setw(8) <<  hgtdRmax[index]
                                << " DeltaZ " << std::setw(8) << hgtdDeltaZ[index] << endmsg;
            }
            msg(MSG::DEBUG) << "HGTD Mother DeltaZ " << std::setw(24) << hgtdDeltaZ[iMother] << endmsg;

            // build up the cooling system of HGTD

            // how many of the volumes to fill and at which index?
            std::vector<unsigned int> cooling_hgtd_collection;
            unsigned int nCoolingTubes = 0;
            for (unsigned int i=0; i<hgtdVolMap.size(); i++ ) {
                if ( (int) hgtdVolMap[i] == iCooling ) {
                    nCoolingTubes += 1;
                    cooling_hgtd_collection.push_back(i);
                }
            }
            if ( nCoolingTubes <= 0 ) {
                throw std::runtime_error("Error in HGTD_DetectorFactory: unable to get HGTD number of cooling tubes!");
            }
            msg(MSG::DEBUG) << "HGTD number of cooling volumes: " << nCoolingTubes << endmsg;

            // count the number of cold tubes
            // the first tube is at y=DeltaY/2
            unsigned int nColdTubes = ((int) (hgtdRmax[iColdTube] - hgtdDeltaY[iColdTube]/2.)/hgtdDeltaY[iColdTube] + 1);
            // now count the number of the tubes cut in half:
            unsigned int nColdTubesHalf = ((int) (hgtdRmin[iColdTube] - hgtdDeltaY[iColdTube]/2.)/hgtdDeltaY[iColdTube] + 1);

            msg(MSG::DEBUG) << "HGTD number of cold tubes " << nColdTubes << endmsg;
            msg(MSG::DEBUG) << "HGTD number of cold tubes halfs " << nColdTubesHalf << endmsg;

            // now the material
            GeoMaterial *material_ColdTubeCoat_hgtd = materialManager->getMaterial(hgtdMaterial[iColdTubeCoat]);
            GeoMaterial *material_ColdTube_hgtd     = materialManager->getMaterial(hgtdMaterial[iColdTube]);

            // position of the cold and coat tubes
            double ycoord0 = hgtdDeltaY[iColdTubeCoat]/2.0;
            std::vector<double> xcoord;
            std::vector<double> ycoord;

            std::vector <double> hgtdColdTubeCoatL;
            std::vector <double> hgtdColdTubeL;

            std::vector <std::string> hgtdColdTubeCoatName;
            std::vector <std::string> hgtdColdTubeName;

            double xOfTube, yOfTube, lengthMin, lengthMax, lengthOfTube;
            // here we fill the calculations (positions, lengths) and names
            for (unsigned int ii = 0; ii<nColdTubes; ii++) {

                yOfTube = ycoord0 + hgtdDeltaY[iColdTubeCoat]*ii;
                ycoord.push_back(yOfTube);
                ycoord.push_back(-yOfTube);

                if (ii < nColdTubesHalf ) {
                    ycoord.push_back(yOfTube);
                    ycoord.push_back(-yOfTube);

                    lengthMax = (sqrt(hgtdRmax[iCooling]*hgtdRmax[iCooling] - (yOfTube+hgtdDeltaZ[iColdTubeCoat])*(yOfTube+hgtdDeltaZ[iColdTubeCoat])));
                    lengthMin = (sqrt(hgtdRmin[iCooling]*hgtdRmin[iCooling] - (yOfTube-hgtdDeltaZ[iColdTubeCoat])*(yOfTube-hgtdDeltaZ[iColdTubeCoat])));

                    xOfTube = lengthMin + (lengthMax - lengthMin)/2.0;
                    xcoord.push_back(xOfTube);
                    xcoord.push_back(xOfTube);
                    xcoord.push_back(-xOfTube);
                    xcoord.push_back(-xOfTube);

                    lengthOfTube = lengthMax - lengthMin;
                    hgtdColdTubeCoatL.push_back(lengthOfTube);
                    hgtdColdTubeCoatL.push_back(lengthOfTube);
                    hgtdColdTubeCoatL.push_back(lengthOfTube);
                    hgtdColdTubeCoatL.push_back(lengthOfTube);

                    hgtdColdTubeL.push_back(lengthOfTube);
                    hgtdColdTubeL.push_back(lengthOfTube);
                    hgtdColdTubeL.push_back(lengthOfTube);
                    hgtdColdTubeL.push_back(lengthOfTube);

                    hgtdColdTubeCoatName.push_back( hgtdName[iColdTubeCoat]+"UpR"+std::to_string(ii) );
                    hgtdColdTubeCoatName.push_back( hgtdName[iColdTubeCoat]+"DownR"+std::to_string(ii) );
                    hgtdColdTubeCoatName.push_back( hgtdName[iColdTubeCoat]+"UpL"+std::to_string(ii) );
                    hgtdColdTubeCoatName.push_back( hgtdName[iColdTubeCoat]+"DownL"+std::to_string(ii) );

                    hgtdColdTubeName.push_back( hgtdName[iColdTube]+"UpR"+std::to_string(ii) );
                    hgtdColdTubeName.push_back( hgtdName[iColdTube]+"DownR"+std::to_string(ii) );
                    hgtdColdTubeName.push_back( hgtdName[iColdTube]+"UpL"+ std::to_string(ii) );
                    hgtdColdTubeName.push_back( hgtdName[iColdTube]+"DownL"+std::to_string(ii) );
                }
                else {
                    xOfTube = 0.;
                    xcoord.push_back(xOfTube);
                    xcoord.push_back(xOfTube);

                    lengthOfTube = 2*(sqrt(hgtdRmax[iCooling]*hgtdRmax[iCooling]-(yOfTube+hgtdDeltaZ[iColdTubeCoat])*(yOfTube+hgtdDeltaZ[iColdTubeCoat])));
                    hgtdColdTubeCoatL.push_back(lengthOfTube);
                    hgtdColdTubeCoatL.push_back(lengthOfTube);

                    hgtdColdTubeL.push_back(lengthOfTube);
                    hgtdColdTubeL.push_back(lengthOfTube);

                    hgtdColdTubeCoatName.push_back( hgtdName[iColdTubeCoat]+"Up"+std::to_string(ii) );
                    hgtdColdTubeCoatName.push_back( hgtdName[iColdTubeCoat]+"Down"+std::to_string(ii) );

                    hgtdColdTubeName.push_back( hgtdName[iColdTube]+"Up"+std::to_string(ii) );
                    hgtdColdTubeName.push_back( hgtdName[iColdTube]+"Down"+std::to_string(ii) );
                }

            } // for (ii = 0;ii<nColdTubes;ii++){

            msg(MSG::DEBUG) << " index hgtdColdTubeCoatName hgtdColdTubeName xcoordinate ycoordinate LengthOfTube LengthOfCoat " << endmsg;
            for (unsigned int i=0; i < hgtdColdTubeName.size(); i++) {
                msg(MSG::DEBUG) << "   " << i
                                << "   " << hgtdColdTubeCoatName[i]
                                << "   " << hgtdColdTubeName[i]
                                << "   " << xcoord[i]
                                << "   " << ycoord[i]
                                << "   " << hgtdColdTubeL[i]
                                << "   " << hgtdColdTubeCoatL[i]
                                << endmsg;
            }

            // the volumes
            std::vector<GeoTube *>    hgtdColdTubeCoatSolidVol;
            std::vector<GeoTube *>    hgtdColdTubeSolidVol;
            std::vector<GeoLogVol *>  hgtdColdTubeCoatLogVol;
            std::vector<GeoLogVol *>  hgtdColdTubeLogVol;
            std::vector<GeoPhysVol *> hgtdColdTubeCoatPhysVol;
            std::vector<GeoPhysVol *> hgtdColdTubePhysVol;

            // now we install the volumes
            for (unsigned int ii = 0;ii < hgtdColdTubeName.size(); ii++) {

                hgtdColdTubeCoatSolidVol.push_back( new GeoTube(0.0,hgtdDeltaZ[iColdTubeCoat],hgtdColdTubeCoatL[ii]/2.0) );
                hgtdColdTubeSolidVol.push_back(     new GeoTube(0.0,hgtdDeltaZ[iColdTube],hgtdColdTubeL[ii]/2.0) );

                // fill into all Cooling units
                for (unsigned int jj = 0; jj < nCoolingTubes;jj++) {
                    // get the index for the cooling volume
                    indexCoolingVolume = cooling_hgtd_collection[jj];
                    // get the index of the last entry in the vectors
                    index = hgtdColdTubeCoatLogVol.size();
                    //Cold tube coating
                    hgtdColdTubeCoatLogVol.push_back( new GeoLogVol(hgtdColdTubeCoatName[ii],hgtdColdTubeCoatSolidVol[ii],material_ColdTubeCoat_hgtd) );
                    hgtdColdTubeCoatPhysVol.push_back( new GeoPhysVol(hgtdColdTubeCoatLogVol[index]) );
                    // attach the coat to to the Cooling volume
                    hgtdPhysVol[indexCoolingVolume]->add(new GeoTransform(HepGeom::Transform3D(CLHEP::HepRotationY(M_PI/2), CLHEP::Hep3Vector(xcoord[ii],ycoord[ii],0.0))));
                    hgtdPhysVol[indexCoolingVolume]->add(hgtdColdTubeCoatPhysVol[index]);
                    //Cold tube
                    hgtdColdTubeLogVol.push_back( new GeoLogVol(hgtdColdTubeName[ii],hgtdColdTubeSolidVol[ii],material_ColdTube_hgtd) );
                    hgtdColdTubePhysVol.push_back( new GeoPhysVol(hgtdColdTubeLogVol[index]) );
                    // attach the coat
                    hgtdColdTubeCoatPhysVol[index]->add(new GeoTransform(HepGeom::TranslateZ3D(0.0)));
                    hgtdColdTubeCoatPhysVol[index]->add(hgtdColdTubePhysVol[index]);
                } // for (jj = 0;jj<nCoolingTubes;jj++) {
            } // for (ii = 0;ii<hgtdColdTubeName.size();ii++) {

        } // end of fullgeo

    } // end of hgtdTiming or hgtdPreshower
#endif
    return world_physical_hgtd;
}

void HGTDPlugin::buildHgtdGeoTDR(    const GeoXmlMatManager* matmanager,
                                           GeoFullPhysVol* HGTDparent, bool isPos) {


    // to be calculated from parameters in db using map
    double motherHalfZ = ((GeoTube*) HGTDparent->getLogVol()->getShape())->getZHalfLength();
    double modulePackageHalfZtot = 3.5/2+4./2; // including flex - can we not get this from the db numbers? /CO

    double modulePackageHalfZ = 2*m_boxVolPars["HGTD::GlueSensor"].zHalf + m_boxVolPars["HGTDSiSensor0"].zHalf
                                + m_boxVolPars["HGTD::LGADInactive"].zHalf + m_boxVolPars["HGTD::ASIC"].zHalf
                                + m_boxVolPars["HGTD::Hybrid"].zHalf + m_boxVolPars["HGTD::ModuleSpace"].zHalf;

    // add volumes by key name to ordered vector, outside in (from larger z to smaller)
    std::vector<std::string> hgtdVolumes;
    hgtdVolumes.push_back("HGTD::ModeratorOut"); // Out as in outside the vessel
    hgtdVolumes.push_back("HGTD::BackCover");
    hgtdVolumes.push_back("HGTD::ToleranceBack");
    hgtdVolumes.push_back("HGTD::ModeratorIn"); // In as in inside the vessel
    hgtdVolumes.push_back("HGTD::ModuleLayer3");
    hgtdVolumes.push_back("HGTD::SupportPlate");
    hgtdVolumes.push_back("HGTD::CoolingPlate");
    hgtdVolumes.push_back("HGTD::SupportPlate");
    hgtdVolumes.push_back("HGTD::ModuleLayer2");
    hgtdVolumes.push_back("HGTD::ToleranceMid");
    hgtdVolumes.push_back("HGTD::ModuleLayer1");
    hgtdVolumes.push_back("HGTD::SupportPlate");
    hgtdVolumes.push_back("HGTD::CoolingPlate");
    hgtdVolumes.push_back("HGTD::SupportPlate");
    hgtdVolumes.push_back("HGTD::ModuleLayer0");
    hgtdVolumes.push_back("HGTD::ToleranceFront");
    hgtdVolumes.push_back("HGTD::FrontCover");
    // Important - these must come last since they will otherwise shift positions of the previous volumes!
    hgtdVolumes.push_back("HGTD::InnerRCover"); // don't reorder!
    hgtdVolumes.push_back("HGTD::OuterRCover"); // don't reorder!

    // Now build up the solid, logical and physical volumes as appropriate (starting from the outermost volume)
    // We first start with the volumes we'll reuse several times

    //////////////////////////
    // FLEX PACKAGE VOLUMES //
    //////////////////////////

    // Flex package volume modeled as 8 concentric flex sheets with progressively larger inner radius
    // Order of sheets depend on whether package is for front or back of a cooling plate
    // First calculate the inner radii for the flex sheets
    GeoCylVolParams packagePars = m_cylVolPars["HGTD::FlexPackage"];
    GeoCylVolParams flexPars = m_cylVolPars["HGTD::FlexTube"];
    std::vector<double> flexSheetInnerR;
    double currentInnerR = 144.; // adding flex sheets from the second sensor (all have the hybrid already)
    for (int flexSheet = 0; flexSheet < 8; flexSheet++) {
        flexSheetInnerR.push_back(currentInnerR);
        // set the inner radius for the next flex sheet, increased by two module heights and two radius-dependent spaces per sheet
        currentInnerR += m_boxVolPars["HGTDModule0"].xHalf*2 * (2 + 2 * (flexSheet < 4 ? 0.2 : 0.8) );
    }

    // build up the two flex volumes for front (0) and back (1) sides
    GeoPhysVol* flexPackagePhysical[2] = {};
    for (int flexVolume = 0; flexVolume < 2; flexVolume++) {

        std::vector<double> rInner = flexSheetInnerR;
        if (flexVolume) reverse(rInner.begin(), rInner.end()); // reverse order for backside flex package

        GeoTube*    flexPackageSolid = new GeoTube(packagePars.rMin, packagePars.rMax, packagePars.zHalf);
        GeoLogVol*  flexPackageLogical = new GeoLogVol(packagePars.name, flexPackageSolid, matmanager->getMaterial(packagePars.material));
        flexPackagePhysical[flexVolume] = new GeoPhysVol(flexPackageLogical);
        // build up a volume of flex cables, starting in z at half a flex layer from the edge of the flex package volume
        double flexZoffset = packagePars.zHalf - flexPars.zHalf;
        for (int flexSheet = 0; flexSheet < 8; flexSheet++) {
            GeoTube*    hgtdFlexSolid    = new GeoTube(rInner[flexSheet], flexPars.rMax, flexPars.zHalf);
            GeoLogVol*  hgtdFlexLogical  = new GeoLogVol("HGTD::FlexTube"+std::to_string(flexSheet),
                                                         hgtdFlexSolid, matmanager->getMaterial(flexPars.material));
            GeoPhysVol* hgtdFlexPhysical = new GeoPhysVol(hgtdFlexLogical);
            flexPackagePhysical[flexVolume]->add(new GeoTransform(GeoTrf::TranslateZ3D(flexZoffset)));
            flexPackagePhysical[flexVolume]->add(hgtdFlexPhysical);
            // print out a line for each flex layer
            flexZoffset = flexZoffset - m_hgtdPars.flexSheetSpacing;
        }

    }

    ///////////////////
    // COOLING TUBES //
    ///////////////////

    // make list of radii of cooling tubes
    std::vector<double> coolingTubeRadii;
    double coolingTubeRadius = 130.;
    coolingTubeRadii.push_back(coolingTubeRadius);
    for (int i = 0; i < 18; i++) {
        coolingTubeRadius += (418-130.)/18;
        coolingTubeRadii.push_back(coolingTubeRadius);
    }
    for (int i = 0; i < 12; i++) {
        coolingTubeRadius += (658-418.)/14;
        coolingTubeRadii.push_back(coolingTubeRadius);
    }
    coolingTubeRadius = 710.;
    coolingTubeRadii.push_back(coolingTubeRadius);
    for (int i = 0; i < 7; i++) {
        coolingTubeRadius += (890-710.)/6;
        coolingTubeRadii.push_back(coolingTubeRadius);
    }
    std::string tmp = "Cooling tubes will be created at the following radii (in mm):";
    for (size_t i = 0; i << coolingTubeRadii.size(); i++) {
        tmp += (std::string(" ") + std::to_string(coolingTubeRadii[i]));
    }

    ///////////////////////////////////
    // PERIPHERAL ELECTRONICS VOLUME //
    ///////////////////////////////////

    //build peripheral electronics
    GeoCylVolParams periphElPars = m_cylVolPars["HGTD::PeriphElec"];
    GeoTube*    periphElec_solid  = new GeoTube(periphElPars.rMin, periphElPars.rMax, periphElPars.zHalf);
    GeoLogVol*  periphElec_log    = new GeoLogVol(periphElPars.name, periphElec_solid, matmanager->getMaterial(periphElPars.material));
    GeoPhysVol* periphElec_phys   = new GeoPhysVol(periphElec_log);

    GeoPhysVol* moduleLayerPhysical[4] = {}; // array of pointers to the physical volumes for the module layers which need special care

    ///////////////////////////////////////////
    // BUILD UP ALL MAIN VOLUMES IN SEQUENCE //
    ///////////////////////////////////////////

    // now build up the volumes in the order specified in the vector
    double zModuleLayerF = 0.;
    double zModuleLayerB = 0.;
    for (size_t vol = 0; vol < hgtdVolumes.size(); vol++) {

        std::string v = hgtdVolumes[vol];
        // calculate local z offsets for each main volume sequentially
        if (vol == 0) // special treatment for the first one
            m_cylVolPars[v].zOffsetLocal = motherHalfZ - m_cylVolPars[v].zHalf;
        // All but the InnerRCover and OuterRCover are placed relative to other components,
        // but the zOffsetLocal parameter of these two volumes is left as read from the db
        else if (v.substr(9,16) != "erRCover") {
            std::string vPrev = hgtdVolumes[vol-1];
            m_cylVolPars[v].zOffsetLocal = m_cylVolPars[vPrev].zOffsetLocal - m_cylVolPars[vPrev].zHalf - m_cylVolPars[v].zHalf;
        }
        GeoTube*    hgtdSubVolumeSolid    = new GeoTube(m_cylVolPars[v].rMin, m_cylVolPars[v].rMax, m_cylVolPars[v].zHalf);
        GeoLogVol*  hgtdSubVolumeLogical  = new GeoLogVol(m_cylVolPars[v].name, hgtdSubVolumeSolid,
                                                          matmanager->getMaterial(m_cylVolPars[v].material));
        GeoPhysVol* hgtdSubVolumePhysical = new GeoPhysVol(hgtdSubVolumeLogical);

        // if building the cooling plate, also add peripheral electronics since position of those are relative to that of cooling plate
        if (v == "HGTD::CoolingPlate") {
            double zOffsetPeriphElec = m_cylVolPars[v].zHalf + periphElPars.zOffsetLocal + periphElPars.zHalf;
            for (int side = 0; side < 2; side++) { // place two, one on each side of cooling plate
                HGTDparent->add(new GeoTransform(GeoTrf::TranslateZ3D(m_cylVolPars[v].zOffsetLocal + pow(-1, side)*zOffsetPeriphElec)));
                HGTDparent->add(periphElec_phys);
            }

            // and the CO2 cooling tubes inside the cooling plate
            for (size_t i = 0; i < coolingTubeRadii.size(); i++) {
                GeoTorus* coolingTubeSolid = new GeoTorus(m_cylVolPars["HGTD::CoolingTubeFluid"].zHalf, m_cylVolPars["HGTD::CoolingTube"].zHalf,
                                                          coolingTubeRadii[i], 0, 2*M_PI);
                GeoLogVol* coolingTubeLogical = new GeoLogVol("HGTD::CoolingTube", coolingTubeSolid,
                                                              matmanager->getMaterial(m_cylVolPars["HGTD::CoolingTube"].material));
                GeoPhysVol* coolingTubePhysical = new GeoPhysVol(coolingTubeLogical);
                hgtdSubVolumePhysical->add(coolingTubePhysical); // no transformations needed, concentric with cooling plate and centered in z
                // and the contents, i.e. the cooling fluid
                GeoTorus* coolingFluidSolid = new GeoTorus(0, m_cylVolPars["HGTD::CoolingTubeFluid"].zHalf,
                                                           coolingTubeRadii[i], 0, 2*M_PI);
                GeoLogVol* coolingFluidLogical = new GeoLogVol("HGTD::CoolingFluid", coolingFluidSolid,
                                                               matmanager->getMaterial(m_cylVolPars["HGTD::CoolingTubeFluid"].material));
                GeoPhysVol* coolingFluidPhysical = new GeoPhysVol(coolingFluidLogical);
                hgtdSubVolumePhysical->add(coolingFluidPhysical); // no transformations needed, concentric with cooling plate and centered in z
            }
        }

        // module layer
        if (v.substr(0,17) == "HGTD::ModuleLayer") {

            int layer = atoi(v.substr(17,1).c_str());

            // front and back side layers are treated differently: z position of flex and module layers, and rotation
            double zFlex = 0.;
            bool Lside = layer % 2;
            if (Lside == 0) { // layers 0 and 2
                zFlex = -modulePackageHalfZtot + m_cylVolPars["HGTD::FlexPackage"].zHalf;
                zModuleLayerF = modulePackageHalfZtot - modulePackageHalfZ;
            }
            else { // layers 1 and 3
                zFlex = modulePackageHalfZtot - m_cylVolPars["HGTD::FlexPackage"].zHalf;
                zModuleLayerB = -modulePackageHalfZtot + modulePackageHalfZ;
            }

            // place flex within module packages, at different positions depending on front or back or cooling plate
            hgtdSubVolumePhysical->add(new GeoTransform(GeoTrf::TranslateZ3D(zFlex)));
            hgtdSubVolumePhysical->add(flexPackagePhysical[(Lside ? 0 : 1)]);

            // place module layer volumes
            int rotationSign = (layer <= 1 ? 1 : -1);
            HGTDparent->add(new GeoTransform(GeoTrf::TranslateZ3D(m_cylVolPars[v].zOffsetLocal)*GeoTrf::RotateZ3D(rotationSign*m_hgtdPars.diskRotation*SYSTEM_OF_UNITS::deg)));
            HGTDparent->add(hgtdSubVolumePhysical);
            moduleLayerPhysical[layer] = hgtdSubVolumePhysical;

        } // end of module package

        else {
            HGTDparent->add(new GeoTransform(GeoTrf::TranslateZ3D(m_cylVolPars[v].zOffsetLocal)));
            HGTDparent->add(hgtdSubVolumePhysical);
        }

    } // end loop over hgtdVolumes

    ////////////////////
    // MODULE VOLUMES //
    ////////////////////

    // create the module --> each for cell and with different names
    // calculate the positions where modules should be placed in one quadrant
    std::vector<std::vector<ModulePosition> > positions;
    positions.push_back(calculateHgtdModulePositionsInQuadrant(0)); // front-side module positions
    positions.push_back(calculateHgtdModulePositionsInQuadrant(1)); // back-side module positions

    // components for the module
    std::vector<std::string> moduleVolumes;
    moduleVolumes.push_back("HGTD::GlueAsic");
    moduleVolumes.push_back("HGTD::ASIC");
    moduleVolumes.push_back("HGTD::LGADInactive");
    moduleVolumes.push_back("SensorPlaceHolder"); // replaced below to get the numbered name right
    moduleVolumes.push_back("HGTD::GlueSensor");
    moduleVolumes.push_back("HGTD::Hybrid");
    if (m_boxVolPars["HGTD::ModuleSpace"].zHalf) moduleVolumes.push_back("HGTD::ModuleSpace");

    int endcap_side   = isPos ? +2 : -2;
    int layer_offset  = 5;


    // loop over layers
    for (int layer = 0; layer < 4; layer++) {
        // select if you need the front or the back positions
        int front_back = layer % 2;
        bool isFront   = (front_back == 0) ? true : false;

        std::vector<std::string> volumes = moduleVolumes;
        if (not isFront) reverse(volumes.begin(), volumes.end()); // reverse order of components for backside modules

        std::string sensorName = std::string("HGTDSiSensor")+std::to_string(layer);
        std::string moduleName = std::string("HGTDModule")+std::to_string(layer);

        double moduleHalfWidth  = m_boxVolPars[moduleName].yHalf;
        double moduleHalfHeight = m_boxVolPars[moduleName].xHalf;

        // select if you need the front or the back positions
        unsigned int nModules = positions[front_back].size();
        int   max_rows        = 18;

        // loop over quadrants in the current layer
        for (int q = 0; q < 4; q++) {
            // loop over modules in this quadrant
            for (unsigned int element = 0; element < nModules; element++) {
                double x   = positions[front_back].at(element).x;
                double y   = positions[front_back].at(element).y;
                double rot = positions[front_back].at(element).flipped ? 90. : 0.;
                double row = positions[front_back].at(element).row;

                double myphi  = atan(y/x);
                double radius = sqrt(x*x+y*y);

                // module position
                double myx = radius*cos(q*M_PI*0.5+myphi);
                double myy = radius*sin(q*M_PI*0.5+myphi);

                // mirror everything if layer 2 or 3
                double moduleRotation = 0;
                if (layer > 1) {
                    myx = -myx;
                    // need to rotate 180 degrees some modules in q0 and q2
                    if (q%2 == 0 && row <= 15) moduleRotation = 180;
                    else if (q%2 == 1 && row > 15) moduleRotation = 180;
                }

                // these aren't really eta and phi coordinates, misusing names due to borrowing pixel framework
                int eta = (q*max_rows) + positions[front_back].at(element).row;
                int phi = positions[front_back].at(element).el_in_row;

                std::string module_string = "_layer_"+std::to_string(layer_offset+layer)+"_"+std::to_string(phi)+"_"+std::to_string(eta);

                double myModuleHalfHeight = moduleHalfHeight;
                double myModuleHalfWidth  = moduleHalfWidth;

                GeoBox      *moduleSolid    = new GeoBox(myModuleHalfHeight, myModuleHalfWidth, modulePackageHalfZ);
                GeoLogVol   *moduleLogical  = new GeoLogVol(moduleName+module_string, moduleSolid, matmanager->getMaterial("std::Air"));
                GeoFullPhysVol* modulePhysical = new GeoFullPhysVol(moduleLogical);


                // loop over components in module
                for (size_t comp = 0; comp < volumes.size(); comp++) {
                    if (volumes[comp] == "SensorPlaceHolder") volumes[comp] = sensorName; // replace placeholder
                    std::string c = volumes[comp];
                    // calculate local z offsets for each sensor component sequentially, and x offsets for those components that are smaller
                    if (comp == 0) // special treatment for the first one
                        m_boxVolPars[c].zOffsetLocal = modulePackageHalfZ - m_boxVolPars[c].zHalf;
                    else {
                        std::string cPrev = volumes[comp-1];
                        m_boxVolPars[c].zOffsetLocal = m_boxVolPars[cPrev].zOffsetLocal - m_boxVolPars[cPrev].zHalf - m_boxVolPars[c].zHalf;
                    }

                    double comp_halfx = m_boxVolPars[c].xHalf;
                    double comp_halfy = m_boxVolPars[c].yHalf;

                    double xOffsetLocal = myModuleHalfHeight - comp_halfx; // to make room for wire bond of flex to ASIC which is larger than the sensor

                    GeoBox*     sensorCompSolidVol    = new GeoBox(comp_halfx, comp_halfy, m_boxVolPars[c].zHalf);
                    std::string attach = (volumes[comp] == sensorName) ? "" : module_string;

                    GeoLogVol*  sensorCompLogicalVol  = new GeoLogVol(m_boxVolPars[c].name+attach, sensorCompSolidVol, matmanager->getMaterial(m_boxVolPars[c].material));
                    GeoFullPhysVol* sensorCompPhysicalVol = new GeoFullPhysVol(sensorCompLogicalVol);

                    // each SiSensor then has a detector element
                    if (volumes[comp] == sensorName) {
                        GeoTrf::Transform3D sensorTransform = GeoTrf::TranslateZ3D(m_boxVolPars[c].zOffsetLocal)*
                                                               GeoTrf::TranslateX3D(xOffsetLocal);
                        GeoAlignableTransform* xform = new GeoAlignableTransform(sensorTransform);
                        modulePhysical->add(xform);
                        modulePhysical->add(sensorCompPhysicalVol);
                    } else {
                        modulePhysical->add(new GeoTransform(GeoTrf::TranslateZ3D(m_boxVolPars[c].zOffsetLocal)*GeoTrf::TranslateX3D(xOffsetLocal)));
                        modulePhysical->add(sensorCompPhysicalVol);
                    }

                } // end of components loop

                double zModule = isFront ? zModuleLayerF : zModuleLayerB;
                GeoTransform * moduleTransform = new GeoTransform(GeoTrf::TranslateZ3D(zModule)*
                                                                  GeoTrf::TranslateX3D(myx)*
                                                                  GeoTrf::TranslateY3D(myy)*
                                                                  GeoTrf::RotateZ3D((rot+q*90+moduleRotation)*SYSTEM_OF_UNITS::deg));
                moduleLayerPhysical[layer]->add(moduleTransform);
                moduleLayerPhysical[layer]->add(modulePhysical);

            } //end of modules loop

        } // end of quadrants loop

    } // end of layers loop

}


std::vector<HGTDPlugin::ModulePosition> HGTDPlugin::calculateHgtdModulePositionsInQuadrant(int layer) {
    std::vector<ModulePosition> modulePositions;
    for (size_t row = 0; row < 18; row++) {
        bool back = (layer % 2);
        std::vector<ModulePosition> rowModulePositions = calculateHgtdModulePositionsInRow(row, back);
        modulePositions.insert(modulePositions.end(), rowModulePositions.begin(), rowModulePositions.end());
    }
    return modulePositions;
}


std::vector<HGTDPlugin::ModulePosition> HGTDPlugin::calculateHgtdModulePositionsInRow(int row, bool back) {

    unsigned int module = 0;
    std::vector<ModulePosition> modulePositions;
    double posOfLastPlacedModule = 0.;

    while (true) {
        //msg(MSG::INTO) << "Will now place module " << module << endmsg;
        // horizontal rows need care (restart from other edge of quadrant), this variable helps get the inner radius right
        // in quadrant 0 - ie top right quadrant
        // row 0 = bottom horizontal row. numbering grows upwards and counterclockwise; row 17=leftmost vertical row
        // rowForInnerRadius = 0-1 for vertical rows too
        int rowForInnerRadius = row; // because row 16-17 are equivalent to 0-1 regarding module placement
        if (row == 17) rowForInnerRadius = 0;
        if (row == 16) rowForInnerRadius = 1;

        // params needed frequently below
        double moduleWidth = m_boxVolPars["HGTDModule0"].yHalf*2;
        double moduleHeight = m_boxVolPars["HGTDModule0"].xHalf*2;
        double rInner = m_cylVolPars["HGTD::ModuleLayer0"].rMin;
        double rMid = m_hgtdPars.rMid;
        double rOuter = m_hgtdPars.rOuter;
        double rowSpaceSide = m_hgtdPars.rowSpaceSide;

        // x coordinate for vertical rows
        double rowCenterPos = (moduleWidth + rowSpaceSide)*(rowForInnerRadius + 0.5);
        // y coordinate for vertical rows
        double modulePosAlongRow = -99.; // mock value for now
        // for the first module, pick the right starting point
        if (module == 0) {
            // start at inner radius and include any offset for backside.
            if (rowForInnerRadius < 3) {
                modulePosAlongRow = sqrt( pow(rInner, 2) - pow((moduleWidth + rowSpaceSide)*rowForInnerRadius, 2) )
                                    + back*m_hgtdPars.rowBacksideInnerShift + moduleHeight/2;
            }
            else { // later modules start at 2*moduleWidth, with offset for the backside
                double backSpacing = m_hgtdPars.rowBacksideInnerShift;
                if (back && (rowCenterPos - moduleWidth/2 > rMid)) {
                    backSpacing = m_hgtdPars.rowBacksideOuterShift;
                }
                modulePosAlongRow = 2*(moduleWidth + rowSpaceSide) + moduleHeight/2 + back*backSpacing;
            }
        }
        // the rest of the modules follow sequential, radius-dependent placement rules
        else {
            ModulePosition prev = modulePositions.back();
            double spacing = m_hgtdPars.moduleSpaceInner;
            // if the previous module was completely outside rMid, increase the spacing
            // (+1 mm is a needed shift for full coverage - could need tweaking if layout parameters change!)
            float innermostCornerR = sqrt( pow(prev.y - moduleHeight/2, 2) + pow(prev.x - moduleWidth/2, 2) ) + 1.;
            if (innermostCornerR > rMid) {
                spacing = m_hgtdPars.moduleSpaceOuter;
            }
            // for the back the large spacing starts as soon as the space would entirely be outside R = 320 mm
            if (back) {
                double startOfSpaceAlongRow = sqrt( pow(prev.y + moduleHeight/2, 2) + pow(prev.x - moduleWidth/2, 2)) - 2;
                if (startOfSpaceAlongRow > rMid) {
                    spacing = m_hgtdPars.moduleSpaceOuter;
                }
            }
            // correction to the first two spaces on the front side, compensating for the 2 mm introduced at beginning of a row
            if (!back && rowForInnerRadius < 8 && module < 3) {
                spacing -= 1.; // should result in 3 mm instead of 4 mm for innermost spaces on rows starting at R = 120
            }
            // squeeze in a few more modules at the end of some rows
            double maxRcut = rOuter+20;
            if(row == 8 || row == 9 || row == 10) {
                maxRcut = 661;
                if(row == 8 && module > 12) spacing -= 4;
            }
            else if ( row == 11 && module > 9) {
                maxRcut = 662;
                spacing -= 6;
            }
            else if (row == 12 && back) {
                maxRcut = 665;
            }
            else if(row == 13 && module > 5) {
                maxRcut = 666;
                if (!back && module > 6 ) spacing -= 8.5;
                else if (back && module > 5) spacing -= 2;
            }
            else if (row == 14 && module > 3) {
                maxRcut = 665;
                spacing -= 5;
            }
            else if (row == 15) {
                maxRcut = 669;
                spacing -= 5.5;
            }
            modulePosAlongRow = posOfLastPlacedModule + moduleHeight + spacing;
            // stop if the next module will extend outside the max allowed radius
            // HC max radius is 665
            if (sqrt( pow(rowCenterPos + moduleWidth/2, 2) + pow(modulePosAlongRow + moduleHeight/2, 2) ) > maxRcut) {
                break;
            }
        }
        ModulePosition m = {rowCenterPos, modulePosAlongRow, 0, true, row, (int)module};
        modulePositions.push_back(m);
        posOfLastPlacedModule = modulePosAlongRow;
        module += 1;
    } // end of loop over modules

    // finally, flip x and y for all modules if this row is horizontal
    if (row < 16) {
        // msg(MSG::INFO) << "Flipping x and y for modules in row " << row << endmsg;
        for (size_t i=0; i < modulePositions.size(); i++) {
            ModulePosition old = modulePositions[i];
            ModulePosition rotated = old;
            rotated.x = old.y;
            rotated.y = old.x;
            rotated.flipped = !old.flipped;
            rotated.row = old.row;
            rotated.el_in_row = old.el_in_row;
            modulePositions[i] = rotated;
        }
    }

    //std::cout << "row = " << row << std::endl;
    //for(size_t i=0; i < modulePositions.size(); i++) {
    //    std::cout << "Module " << i << " at (x,y) = (" << modulePositions[i].x << "," << modulePositions[i].y << ")" << std::endl;
    //}

    return modulePositions;
}
