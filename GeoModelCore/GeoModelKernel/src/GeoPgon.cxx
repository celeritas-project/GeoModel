/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoModelKernel/GeoPgon.h"
#include "GeoModelKernel/GeoShapeAction.h"
#include <cmath>
#include <stdexcept>

const std::string GeoPgon::s_classType = "Pgon";
const ShapeType GeoPgon::s_classTypeID = 0x14;

GeoPgon::GeoPgon (double SPhi, double DPhi, unsigned int NSides)
  : m_sPhi (SPhi)
  , m_dPhi (DPhi)
  , m_nSides (NSides)
{
}

GeoPgon::~GeoPgon()
{
}

double GeoPgon::volume () const
{
  if (!isValid ())
    throw std::runtime_error ("Volume requested for incomplete polygon");
  double v = 0.0;
  for (size_t k = 0; k < getNPlanes() - 1; ++k) {
    double z1 = getZPlane(k);
    double z2 = getZPlane(k + 1);
    double a1 = getRMinPlane(k);
    double a2 = getRMinPlane(k + 1);
    double b1 = getRMaxPlane(k);
    double b2 = getRMaxPlane(k + 1);
    v += (b1*b1 + b1*b2 + b2*b2 - a1*a1 - a1*a2 - a2*a2) * (z2 - z1);
  }
  int nsides = getNSides();
  double alpha = m_dPhi/nsides;
  double sinAlpha = sin(alpha);
  double cosHalfAlpha = cos(0.5 * alpha);
  return fabs(v) * nsides * sinAlpha / (cosHalfAlpha * cosHalfAlpha * 6.0);
}

const std::string & GeoPgon::type () const
{
  return s_classType;
}

ShapeType GeoPgon::typeID () const
{
  return s_classTypeID;
}

void GeoPgon::addPlane (double ZPlane, double RMinPlane, double RMaxPlane)
{
  m_zPlane.push_back (ZPlane);
  m_rMinPlane.push_back (RMinPlane);
  m_rMaxPlane.push_back (RMaxPlane);
}

void GeoPgon::exec (GeoShapeAction *action) const
{
  action->handlePgon(this);
}
