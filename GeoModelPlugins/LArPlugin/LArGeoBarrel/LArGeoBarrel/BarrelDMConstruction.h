/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file BarrelDMConstruction.h
 *
 * @brief Declaration of BarrelDMConstruction class
 *
 */

#ifndef LARGEOBARREL_BARRELDMCONSTRUCTION_H
#define LARGEOBARREL_BARRELDMCONSTRUCTION_H

class GeoFullPhysVol;

namespace LArGeo {
  
  /** @class LArGeo::BarrelDMConstruction
      @brief GeoModel description of dead material in the crack region
  */
  class BarrelDMConstruction
  {
  public:
    BarrelDMConstruction(bool activateFT = false);
    virtual ~BarrelDMConstruction();

    // Add passive materials to the barrel envelope
    void create(GeoFullPhysVol* envelope);

  private:
    // It is illegal to copy/assign a BarrelDMConstruction:
    BarrelDMConstruction (const BarrelDMConstruction &) = delete;
    BarrelDMConstruction & operator= (const BarrelDMConstruction &) = delete;
    
    bool m_activateFT;
  };
}  // namespace LArGeo

#endif // LARGEOBARREL_BARRELDMCONSTRUCTION_H
