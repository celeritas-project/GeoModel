/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoModelKernel/GeoTrd.h"
#include "GeoModelKernel/GeoShapeAction.h"

const std::string GeoTrd::s_classType = "Trd";
const ShapeType GeoTrd::s_classTypeID = 0x16;

GeoTrd::GeoTrd (double XHalfLength1, double XHalfLength2, double YHalfLength1, double YHalfLength2, double ZHalfLength)
  : m_xHalfLength1 (XHalfLength1)
  , m_xHalfLength2 (XHalfLength2)
  , m_yHalfLength1 (YHalfLength1)
  , m_yHalfLength2 (YHalfLength2)
  , m_zHalfLength (ZHalfLength)
{}

GeoTrd::~GeoTrd()
{}

double GeoTrd::volume () const
{
  double fDz = m_zHalfLength;
  double fDy1 = m_yHalfLength1;
  double fDx1 = m_xHalfLength1;
  double fDx2 = m_xHalfLength2;
  double fDy2 = m_yHalfLength2;
  return 2.0 * fDz *
    ((fDx1 + fDx2) * (fDy1 + fDy2) + (fDx2 - fDx1) * (fDy2 - fDy1) * (1./3.));
}

const std::string & GeoTrd::type () const
{
  return s_classType;
}

ShapeType GeoTrd::typeID () const
{
  return s_classTypeID;
}

void GeoTrd::exec (GeoShapeAction *action) const
{
  action->handleTrd(this);
}

