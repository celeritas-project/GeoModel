/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoModelKernel/GeoTrap.h"
#include "GeoModelKernel/GeoShapeAction.h"

const std::string GeoTrap::s_classType = "Trap";
const ShapeType GeoTrap::s_classTypeID = 0x15;

GeoTrap::GeoTrap (double ZHalfLength, double Theta, double Phi, double Dydzn, double Dxdyndzn, double Dxdypdzn, double Angleydzn, double Dydzp, double Dxdyndzp, double Dxdypdzp, double Angleydzp)
  : m_zHalfLength (ZHalfLength)
  , m_theta (Theta)
  , m_phi (Phi)
  , m_dydzn (Dydzn)
  , m_dxdyndzn (Dxdyndzn)
  , m_dxdypdzn (Dxdypdzn)
  , m_angleydzn (Angleydzn)
  , m_dydzp (Dydzp)
  , m_dxdyndzp (Dxdyndzp)
  , m_dxdypdzp (Dxdypdzp)
  , m_angleydzp (Angleydzp)
{
}

GeoTrap::~GeoTrap()
{
}

double GeoTrap::volume () const
{
  double fDz  = m_zHalfLength;
  double fDy1 = m_dydzn;
  double fDx1 = m_dxdyndzn;
  double fDx2 = m_dxdypdzn;
  double fDy2 = m_dydzp;
  double fDx3 = m_dxdyndzp;
  double fDx4 = m_dxdypdzp;
  return fDz * ((fDx1 + fDx2 + fDx3 + fDx4) * (fDy1 + fDy2) +
                (fDx4 + fDx3 - fDx2 - fDx1) * (fDy2 - fDy1) * (1./3.));
}

const std::string & GeoTrap::type () const
{
  return s_classType;
}

ShapeType GeoTrap::typeID () const
{
  return s_classTypeID;
}

void GeoTrap::exec (GeoShapeAction *action) const
{
  action->handleTrap(this);
}
