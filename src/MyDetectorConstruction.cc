
#include "MyDetectorConstruction.hh"

#include "globals.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"

#include "G4UniformMagField.hh"
#include "G4FieldManager.hh"
#include "G4TransportationManager.hh"
#include "G4VPhysicalVolume.hh"
#include "G4RunManager.hh"
#include "MyDetectorMessenger.hh"
#include "G4PVPlacement.hh"

// **** INCLUDES for GeoModel
#include "GeoModelDBManager/GMDBManager.h"
#include "GeoModelRead/ReadGeoModel.h"
#include "GeoModel2G4/ExtParameterisedVolumeBuilder.h"
#include "G4LogicalVolume.hh"
#include "G4UnitsTable.hh"

#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoNameTag.h"

#include "GeoModelKernel/GeoVGeometryPlugin.h"
#include "GeoModelKernel/GeoGeometryPluginLoader.h"

#include <QCoreApplication>
#include <QString>
#include <QDebug>
#include <QFileInfo>

// Units
#include "GeoModelKernel/Units.h"
#define SYSTEM_OF_UNITS GeoModelKernelUnits // so we will get, e.g., 'GeoModelKernelUnits::cm'
// ****

G4double MyDetectorConstruction::gFieldValue = 0.0;

MyDetectorConstruction::MyDetectorConstruction() : fWorld(nullptr), fDetectorMessenger(nullptr)
{
  fGeometryFileName    = "geometry-ATLAS-R2-2016-01-00-01.db";
  fFieldValue          = 0.0;
  fDetectorMessenger   = new MyDetectorMessenger(this);
  fRunOverlapCheck     = false;
  fReportFileName      = "gmclash_report.json";
}

MyDetectorConstruction::~MyDetectorConstruction()
{
  delete fDetectorMessenger;
}

void MyDetectorConstruction::RecursivelyCheckOverlap(G4LogicalVolume* envelope){
    
    int localNoDaughters = envelope->GetNoDaughters();
    //std::cout<<"Total n. of Daughters of "<<envelope->GetName()<<" is : "<<localNoDaughters<<std::endl;
    for (int sampleNo=0; sampleNo<localNoDaughters; sampleNo++){
        
        G4VPhysicalVolume *daughter=envelope->GetDaughter(sampleNo);
        if(daughter->GetLogicalVolume()->GetNoDaughters()>0)
                RecursivelyCheckOverlap(daughter->GetLogicalVolume());
        //std::cout<<"Starting Overlaps check on daughter: "<<daughter->GetName()<<std::endl;
        //daughter->CheckOverlaps();
        myCheckOverlaps(daughter);
    }
}

bool MyDetectorConstruction::myCheckOverlaps(G4VPhysicalVolume* volume, G4int res, G4double tol,
                                           G4bool verbose, G4int maxErr)
{
    
        if (res <= 0) { return false; }
        G4VSolid* solid = volume->GetLogicalVolume()->GetSolid();
        G4LogicalVolume* motherLog = volume->GetMotherLogical();
        if (motherLog == nullptr) {
            std::cout<<"motherLog == nullptr!!!! :,(((( "<<std::endl;
            std::cout<<"solid->getName(): "<<solid->GetName()<<std::endl;
            exit(-1);
            return false; }
        
        G4int trials = 0;
        G4bool retval = false;
        
        if (verbose)
        {
            G4cout << "Checking overlaps for volume " << volume->GetName()
            << " (" << solid->GetEntityType() << ") ... ";
        }

        // Check that random points are gererated correctly
        //
        G4ThreeVector ptmp = solid->GetPointOnSurface();
        if (solid->Inside(ptmp) != kSurface)
        {
            G4String position[3] = { "outside", "surface", "inside" };
            std::ostringstream message;
            message << "Sample point is not on the surface !" << G4endl
            << "          The issue is detected for volume "
            << volume->GetName() << ':' << volume->GetCopyNo()
            << " (" << solid->GetEntityType() << ")" << G4endl
            << "          generated point " << ptmp
            << " is " << position[solid->Inside(ptmp)];
            G4Exception("G4PVPlacement::CheckOverlaps()",
                        "GeomVol1002", JustWarning, message);
            return false;
        }
        
        // Generate random points on the surface of the solid,
        // transform them into the mother volume coordinate system
        // and find the bonding box
        //
        std::vector<G4ThreeVector> points(res);
        G4double xmin =  kInfinity, ymin =  kInfinity, zmin =  kInfinity;
        G4double xmax = -kInfinity, ymax = -kInfinity, zmax = -kInfinity;
        G4AffineTransform Tm(volume->GetRotation(), volume->GetTranslation());
        for (G4int i = 0; i < res; ++i)
        {
            points[i] = Tm.TransformPoint(solid->GetPointOnSurface());
            xmin = std::min(xmin, points[i].x());
            ymin = std::min(ymin, points[i].y());
            zmin = std::min(zmin, points[i].z());
            xmax = std::max(xmax, points[i].x());
            ymax = std::max(ymax, points[i].y());
            zmax = std::max(zmax, points[i].z());
        }
        
        // Check overlap with the mother volume
        //
        G4VSolid* motherSolid = motherLog->GetSolid();
        for (G4int i = 0; i < res; ++i)
        {
            G4ThreeVector mp = points[i];
            if (motherSolid->Inside(mp) != kOutside) continue;
            G4double distin = motherSolid->DistanceToIn(mp);
            if (distin > tol)
            {
                ++trials;
                retval = true;
                std::ostringstream message;
                message << "Overlap with mother volume !" << G4endl
                << "          Overlap is detected for volume "
                << volume->GetName() << ':' << volume->GetCopyNo()
                << " (" << solid->GetEntityType() << ")" << G4endl
                << "          with its mother volume " << motherLog->GetName()
                << " (" << motherSolid->GetEntityType() << ")" << G4endl
                << "          at mother local point " << mp << ", "
                << "overlapping by at least: "
                << G4BestUnit(distin, "Length");
                if (trials >= maxErr)
                {
                    message << G4endl
                    << "NOTE: Reached maximum fixed number -" << maxErr
                    << "- of overlaps reports for this volume !";
                }
                G4Exception("G4PVPlacement::CheckOverlaps()",
                            "GeomVol1002", JustWarning, message);
                if (trials >= maxErr)  { return true; }
                break;
            }
        }
        
        // Checking overlaps with each 'sister' volume
        //
        for (size_t k = 0; k < motherLog->GetNoDaughters(); ++k)
        {
            G4VPhysicalVolume* daughter = motherLog->GetDaughter(k);
            if (daughter == volume) continue;
            G4VSolid* daughterSolid = daughter->GetLogicalVolume()->GetSolid();
            G4AffineTransform Td(daughter->GetRotation(), daughter->GetTranslation());
            
            G4double distout = -kInfinity;
            G4ThreeVector plocal;
            if (!Td.IsRotated()) {
                G4ThreeVector offset = Td.NetTranslation();
                G4ThreeVector pmin, pmax;
                daughterSolid->BoundingLimits(pmin, pmax);
                pmin += offset;
                pmax += offset;
                if (pmin.x() >= xmax) continue;
                if (pmin.y() >= ymax) continue;
                if (pmin.z() >= zmax) continue;
                if (pmax.x() <= xmin) continue;
                if (pmax.y() <= ymin) continue;
                if (pmax.z() <= zmin) continue;
                for (G4int i = 0; i < res; ++i)
                {
                    G4ThreeVector p = points[i];
                    if (p.x() <= pmin.x()) continue;
                    if (p.x() >= pmax.x()) continue;
                    if (p.y() <= pmin.y()) continue;
                    if (p.y() >= pmax.y()) continue;
                    if (p.z() <= pmin.z()) continue;
                    if (p.z() >= pmax.z()) continue;
                    G4ThreeVector md = p - offset;
                    if (daughterSolid->Inside(md) == kInside)
                    {
                        G4double dtmp = daughterSolid->DistanceToOut(md);
                        if (dtmp <= tol) continue;
                        distout = dtmp;
                        plocal = md;
                        break;
                    }
                }
            } else {
                G4ThreeVector pmin, pmax;
                daughterSolid->BoundingLimits(pmin, pmax);
                G4ThreeVector pbox[8] = {
                    G4ThreeVector(pmin.x(), pmin.y(), pmin.z()),
                    G4ThreeVector(pmax.x(), pmin.y(), pmin.z()),
                    G4ThreeVector(pmin.x(), pmax.y(), pmin.z()),
                    G4ThreeVector(pmax.x(), pmax.y(), pmin.z()),
                    G4ThreeVector(pmin.x(), pmin.y(), pmax.z()),
                    G4ThreeVector(pmax.x(), pmin.y(), pmax.z()),
                    G4ThreeVector(pmin.x(), pmax.y(), pmax.z()),
                    G4ThreeVector(pmax.x(), pmax.y(), pmax.z())
                };
                G4double dxmin =  kInfinity, dymin =  kInfinity, dzmin =  kInfinity;
                G4double dxmax = -kInfinity, dymax = -kInfinity, dzmax = -kInfinity;
                for (G4int i = 0; i < 8; ++i)
                {
                    G4ThreeVector p = Td.TransformPoint(pbox[i]);
                    dxmin = std::min(dxmin, p.x());
                    dymin = std::min(dymin, p.y());
                    dzmin = std::min(dzmin, p.z());
                    dxmax = std::max(dxmax, p.x());
                    dymax = std::max(dymax, p.y());
                    dzmax = std::max(dzmax, p.z());
                }
                if (dxmin >= xmax) continue;
                if (dymin >= ymax) continue;
                if (dzmin >= zmax) continue;
                if (dxmax <= xmin) continue;
                if (dymax <= ymin) continue;
                if (dzmax <= zmin) continue;
                for (G4int i = 0; i < res; ++i)
                {
                    G4ThreeVector p = points[i];
                    if (p.x() >= dxmax) continue;
                    if (p.x() <= dxmin) continue;
                    if (p.y() >= dymax) continue;
                    if (p.y() <= dymin) continue;
                    if (p.z() >= dzmax) continue;
                    if (p.z() <= dzmin) continue;
                    G4ThreeVector md = Td.InverseTransformPoint(p);
                    if (daughterSolid->Inside(md) == kInside)
                    {
                        G4double dtmp = daughterSolid->DistanceToOut(md);
                        if (dtmp <= tol) continue;
                        distout = dtmp;
                        plocal = md;
                        break;
                    }
                }
            }
            
            if (distout > tol)
            {
                ++trials;
                retval = true;
                std::ostringstream message;
                message << "Overlap with volume already placed !" << G4endl
                << "          Overlap is detected for volume "
                << volume->GetName() << ':' << volume->GetCopyNo()
                << " (" << solid->GetEntityType() << ")" << G4endl
                << "          with " << daughter->GetName()
                << ':' << daughter->GetCopyNo()
                << " (" << daughterSolid->GetEntityType() << ")"
                << " volume's" << G4endl
                << "          local point " << plocal << ", "
                << "overlapping by at least: "
                << G4BestUnit(distout, "Length");
                if (trials >= maxErr)
                {
                    message << G4endl
                    << "NOTE: Reached maximum fixed number -" << maxErr
                    << "- of overlaps reports for this volume !";
                }
                G4Exception("G4PVPlacement::CheckOverlaps()",
                            "GeomVol1002", JustWarning, message);
                if (trials >= maxErr)  { return true; }
            }
            else if (distout == kInfinity)
            {
                // Now checking that 'sister' volume is not totally included
                // and overlapping. Generate a single point on the surface of
                // the 'sister' volume and verify that the point in NOT inside
                // the current volume
                //
                G4ThreeVector dPoint = daughterSolid->GetPointOnSurface();
                
                // Transform the generated point to the mother's coordinate system
                // and then to current volume's coordinate system
                //
                G4ThreeVector mp2 = Td.TransformPoint(dPoint);
                G4ThreeVector msi = Tm.InverseTransformPoint(mp2);
                
                if (solid->Inside(msi) == kInside)
                {
                    ++trials;
                    retval = true;
                    std::ostringstream message;
                    message << "Overlap with volume already placed !" << G4endl
                    << "          Overlap is detected for volume "
                    << volume->GetName() << ':' << volume->GetCopyNo()
                    << " (" << solid->GetEntityType() << ")" << G4endl
                    << "          apparently fully encapsulating volume "
                    << daughter->GetName() << ':' << daughter->GetCopyNo()
                    << " (" << daughterSolid->GetEntityType() << ")"
                    << " at the same level !";
                    if (trials >= maxErr)
                    {
                        message << G4endl
                        << "NOTE: Reached maximum fixed number -" << maxErr
                        << "- of overlaps reports for this volume !";
                    }
                    G4Exception("G4PVPlacement::CheckOverlaps()",
                                "GeomVol1002", JustWarning, message);
                    if (trials >= maxErr)  { return true; }
                }
            }
        }
        
        if (verbose && trials == 0) { G4cout << "OK! " << G4endl; }
        return retval;
}

GeoPhysVol*  MyDetectorConstruction::CreateTheWorld(GeoPhysVol* world)
{
    if (world == nullptr)
    {
        // Setup the 'World' volume from which everything else will be suspended
        // Get the materials that we shall use.
        // -------------------------------------//
        GeoMaterial* Air=new GeoMaterial("Air", 1.290*SYSTEM_OF_UNITS::mg/SYSTEM_OF_UNITS::cm3);
        GeoElement* Oxigen = new GeoElement("Oxygen",  "O", 8.0, 16.0*SYSTEM_OF_UNITS::g/SYSTEM_OF_UNITS::mole);
        GeoElement* Nitrogen = new GeoElement("Nitrogen", "N", 7., 14.0067*SYSTEM_OF_UNITS::g/SYSTEM_OF_UNITS::mole);
        Air->add(Nitrogen, .8);
        Air->add(Oxigen, .2);
        Air->lock();
        const GeoBox* worldBox = new GeoBox(1000*SYSTEM_OF_UNITS::cm, 1000*SYSTEM_OF_UNITS::cm, 1000*SYSTEM_OF_UNITS::cm);
        const GeoLogVol* worldLog = new GeoLogVol("WorldLog", worldBox, Air);
        world = new GeoPhysVol(worldLog);
    }
    return world;
}

G4VPhysicalVolume *MyDetectorConstruction::Construct()
{
    fTimer.Start();
   
    G4cout << "MyDetectorConstruction::Construct() :: starting the timer"<<G4endl;
    GeoPhysVol* world = nullptr;
    G4LogicalVolume* envelope;
    if (fGeometryFileName.contains(".dylib") || fGeometryFileName.contains(".so"))
    {
        std::cout<< "Bulding the detector from a plugin: "<<fGeometryFileName<<std::endl;
        GeoGeometryPluginLoader loader;
        GeoVGeometryPlugin *factory=loader.load(fGeometryFileName.data());
        if (!factory) {
            std::cout<<"Error!Cannot load geometry from factory. Exiting!"<<std::endl;
            exit(0);
            
        }

        world = CreateTheWorld(nullptr);
        factory->create(world);

        qDebug() << "ReadGeoModel::buildGeoModel() done.";
        fTimer.Stop();
        G4cout << "First step done. GeoModelTree built from the SQLite file."<<G4endl;
        G4cout << "*** Real time elapsed   : " <<fTimer.GetRealElapsed()   << G4endl;
        G4cout << "*** User time elapsed   : " <<fTimer.GetUserElapsed()   << G4endl;
        G4cout << "*** System time elapsed : " <<fTimer.GetSystemElapsed() << G4endl;
        
        fTimer.Start();
        // build the Geant4 geometry and get an hanlde to the world' volume
        ExtParameterisedVolumeBuilder* builder = new ExtParameterisedVolumeBuilder("ATLAS");
        
        std::cout << "Building G4 geometry."<<std::endl;
        envelope = builder->Build(world);
        
        G4VPhysicalVolume* physWorld= new G4PVPlacement(0,G4ThreeVector(),envelope,envelope->GetName(),0,false,0,false);
        
        fWorld = physWorld;
        fWorld->GetLogicalVolume()->SetVisAttributes(G4VisAttributes::Invisible);
        
        if (fWorld == 0) {
            G4ExceptionDescription ed;
            ed << "World volume not set properly check your setup selection criteria or input files!" << G4endl;
            G4Exception("MyDetectorConstruction::Construct()", "FULLSIMLIGHT_0000", FatalException, ed);
        }
        G4cout << "Second step done. Geant4 geometry created from GeoModeltree "<<G4endl;
        G4cout << "Detector Construction from the plugin file " << fGeometryFileName.data() <<", done!"<<G4endl;
        exit(0);
        
    }
    else if (fGeometryFileName.contains(".db")){
        G4cout << "Building the detector from the SQLite file: "<<fGeometryFileName<<G4endl;
        
        // open the DB
        GMDBManager* db = new GMDBManager(fGeometryFileName.data());
        /* Open database */
        if (db->isOpen()) {
            qDebug() << "OK! Database is open!";
        }
        else{
            qDebug() << "Database is not open!";
            // return;
            throw;
            
        }
    
        // -- testing the input database
        //std::cout << "Printing the list of all GeoMaterial nodes" << std::endl;
        //db->printAllMaterials();
        /* setup the GeoModel reader */
        GeoModelIO::ReadGeoModel readInGeo = GeoModelIO::ReadGeoModel(db);
        qDebug() << "ReadGeoModel set.";
        
        
        /* build the GeoModel geometry */
        //GeoPhysVol* world = readInGeo.buildGeoModel(); // builds the whole GeoModel tree in memory and get an handle to the 'world' volume
        world = readInGeo.buildGeoModel(); // builds the whole GeoModel tree in memory and get an handle to the 'world' volume
        qDebug() << "ReadGeoModel::buildGeoModel() done.";
        fTimer.Stop();
        G4cout << "First step done. GeoModelTree built from the SQLite file."<<G4endl;
        G4cout << "*** Real time elapsed   : " <<fTimer.GetRealElapsed()   << G4endl;
        G4cout << "*** User time elapsed   : " <<fTimer.GetUserElapsed()   << G4endl;
        G4cout << "*** System time elapsed : " <<fTimer.GetSystemElapsed() << G4endl;
        
        fTimer.Start();
        // build the Geant4 geometry and get an hanlde to the world' volume
        ExtParameterisedVolumeBuilder* builder = new ExtParameterisedVolumeBuilder("ATLAS");
        
        std::cout << "Building G4 geometry."<<std::endl;
        envelope = builder->Build(world);
        G4VPhysicalVolume* physWorld= new G4PVPlacement(0,G4ThreeVector(),envelope,envelope->GetName(),0,false,0,false);
        
        fWorld = physWorld;
        fWorld->GetLogicalVolume()->SetVisAttributes(G4VisAttributes::Invisible);
        
        if (fWorld == 0) {
            G4ExceptionDescription ed;
            ed << "World volume not set properly check your setup selection criteria or GDML input!" << G4endl;
            G4Exception("MyDetectorConstruction::Construct()", "FULLSIMLIGHT_0000", FatalException, ed);
        }
        G4cout << "Second step done. Geant4 geometry created from GeoModeltree "<<G4endl;
        G4cout << "Detector Construction from the SQLite file " << fGeometryFileName.data() <<", done!"<<G4endl;
    }
    
    else if (fGeometryFileName.contains(".gdml")){
        G4cout << "Building the detector from the GDML file: "<<fGeometryFileName<<G4endl;
        //fParser.SetOverlapCheck(true);
        fParser.Read(fGeometryFileName, false); // turn off schema checker
        fWorld = (G4VPhysicalVolume *)fParser.GetWorldVolume();
        fWorld->GetLogicalVolume()->SetVisAttributes(G4VisAttributes::Invisible);
        
        //RecursivelyCheckOverlap(fWorld->GetLogicalVolume());
        envelope = fWorld->GetLogicalVolume();
        
        if (fWorld == 0) {
            G4ExceptionDescription ed;
            ed << "World volume not set properly! Check your setup selection criteria or the GDML input!" << G4endl;
            G4Exception("MyDetectorConstruction::Construct()", "FULLSIMLIGHT_0001", FatalException, ed);
        }
        G4cout << "Detector Construction from the GDML file " << fGeometryFileName.data() <<", done!"<<G4endl;
        // ConstructSDandField();
        
    }
    else{
        std::cout<< "Error! Geometry format file not supported! Please use one of the following format: .db/.gdml/.so/.dylib. Exiting. "<<std::endl;
        exit(-1);
    }
    
    fTimer.Stop();
    G4cout << "**** Real time elapsed   : " <<fTimer.GetRealElapsed()   << G4endl;
    G4cout << "**** User time elapsed   : " <<fTimer.GetUserElapsed()   << G4endl;
    G4cout << "**** System time elapsed : " <<fTimer.GetSystemElapsed() << G4endl;
    
    if (fRunOverlapCheck){
        RecursivelyCheckOverlap(envelope);
        G4cout<<"Recursive overlap check done! Exiting."<<G4endl;
        exit(0);
    }
    
    return fWorld;
}

void MyDetectorConstruction::ConstructSDandField()
{
  if (std::abs(fFieldValue) > 0.0) {
    // Apply a global uniform magnetic field along the Z axis.
    // Notice that only if the magnetic field is not zero, the Geant4
    // transportion in field gets activated.
    auto uniformMagField     = new G4UniformMagField(G4ThreeVector(0.0, 0.0, fFieldValue));
    G4FieldManager *fieldMgr = G4TransportationManager::GetTransportationManager()->GetFieldManager();
    fieldMgr->SetDetectorField(uniformMagField);
    fieldMgr->CreateChordFinder(uniformMagField);
    G4cout << G4endl << " *** SETTING MAGNETIC FIELD : fieldValue = " << fFieldValue / tesla << " Tesla *** " << G4endl
           << G4endl;

  } else {
    G4cout << G4endl << " *** NO MAGNETIC FIELD SET  *** " << G4endl << G4endl;
  }
}
