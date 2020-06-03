/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4UserActions_LengthIntegrator_H
#define G4UserActions_LengthIntegrator_H

//#include "GaudiKernel/ITHistSvc.h"
//#include "GaudiKernel/ServiceHandle.h"

#include "G4Pow.hh"
#include "TString.h"

#include "G4UserEventAction.hh"
#include "G4UserSteppingAction.hh"

#include <string>
#include <map>

// Forward declarations
class TProfile;
class TProfile2D;


namespace G4UA
{

  /// @class LengthIntegrator
  /// @brief A user action used to evaluate thickness of all detectors
  ///        traversed by outgoing particles.
  ///
  /// This user action is currently used only in special runs with geantinos.
  /// Thickness is recorded in terms of both rad length and int length.
  ///
  /// NOTE: the current design is safe for multi-threading, but _not_
  /// performant due to sharing of the histograms and excessive locking. If
  /// this action needs to be used in multi-threaded jobs, we can rewrite it so
  /// that each instance has its own copy of the histograms which get merged in
  /// finalization of the LengthIntegratorTool.
  ///
  class LengthIntegrator final : public G4UserEventAction,
                                 public G4UserSteppingAction
  {

    public:

      /// Constructor takes the name of the histogram service as argument.
      LengthIntegrator(const std::string& histSvcName);
      /// Destructor
      ~LengthIntegrator();

      /// Called at beginning of G4 event to cache some details about the
      /// current primary vertex and particle. Also resets some measurements.
      virtual void BeginOfEventAction(const G4Event*) override;

      /// Called at end of G4 event to finalize measurements and fill hists
      virtual void EndOfEventAction(const G4Event*) override;

      /// Called at every particle step to accumulate thickness.
      virtual void UserSteppingAction(const G4Step*) override;

    private:

      // Holder for G4 math tools
      G4Pow* m_g4pow;

      // Add elements and values into the map
      void addToDetThickMap(std::string, double, double);

      /// Setup one set of measurement hists for a detector name - ROOT
      void regAndFillHist(const std::string&, const std::pair<double, double>&);
      
      /// Setup one set of measurement hists for a detector name - G4
      void regAndFillHist_g4(const std::string&, const std::pair<double, double>&);

      /// this method checks if a histo is on THsvc already and caches a local pointer to it
      /// if the histo is not present, it creates and registers it
      TProfile2D* getOrCreateProfile(std::string regName, TString histoname, TString xtitle, int nbinsx, float xmin, float xmax,TString ytitle, int nbinsy,float ymin, float ymax,TString ztitle);
      
      G4int getOrCreateProfile_g4(G4String regName, G4String histoname, G4String xtitle, int nbinsx, float xmin, float xmax,G4String ytitle, int nbinsy,float ymin, float ymax,G4String ztitle);

      /// Handle to the histogram service
      //ServiceHandle<ITHistSvc> m_hSvc;

      /// Cached eta of the current primary
      double m_etaPrimary;
      /// Cached phi of the current primary
      double m_phiPrimary;

      /// Map of detector thickness measurements for current event
      std::map<std::string, std::pair<double, double> > m_detThickMap;

      /// Rad-length profile hist in R-Z - ROOT
      TProfile2D* m_rzProfRL;
      /// Rad-length profile hist in eta - ROOT
      std::map<std::string, TProfile*> m_etaMapRL;
      /// Rad-length profile hist in phi - ROOT
      std::map<std::string, TProfile*> m_phiMapRL;

      /// Rad-length profile hist in R-Z - Geant4
      G4int m_rzProfRL_id;
      /// Rad-length profile hist in eta - Geant4
      std::map<std::string, G4int> m_etaMapRL_g4;
      /// Rad-length profile hist in phi - Geant4
      std::map<std::string, G4int> m_phiMapRL_g4;
      
      
      /// Int-length profile hist in R-Z - ROOT
      TProfile2D* m_rzProfIL;
      /// Int-length profile hist in eta - ROOT
      std::map<std::string, TProfile*> m_etaMapIL;
      /// Int-length profile hist in phi - ROOT
      std::map<std::string, TProfile*> m_phiMapIL;
      
      /// Int-length profile hist in R-Z - Geant4
      G4int m_rzProfIL_id;
      /// Int-length profile hist in eta - Geant4
      std::map<std::string, G4int> m_etaMapIL_g4;
      /// Int-length profile hist in phi - Geant4
      std::map<std::string, G4int> m_phiMapIL_g4;

      // 2D plots of rad-length and int-length - ROOT
      std::map<std::string,TProfile2D*,std::less<std::string> > m_rzMapRL;
      std::map<std::string,TProfile2D*,std::less<std::string> > m_xyMapRL;

      std::map<std::string,TProfile2D*,std::less<std::string> > m_rzMapIL;
      std::map<std::string,TProfile2D*,std::less<std::string> > m_xyMapIL;
      
      // 2D plots of rad-length and int-length - Geant4
      std::map<std::string,G4int,std::less<std::string> > m_rzMapRL_g4;
      std::map<std::string,G4int,std::less<std::string> > m_xyMapRL_g4;
      
      std::map<std::string,G4int,std::less<std::string> > m_rzMapIL_g4;
      std::map<std::string,G4int,std::less<std::string> > m_xyMapIL_g4;

  }; // class LengthIntegrator

} // namespace G4UA

#endif

