/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include <cmath>
#include "GeoModelKernel/GeoDefinitions.h"
#include "GeoModelKernel/GeoShapeAction.h"
#include "GeoModelKernel/GeoPara.h"

const std::string GeoPara::s_classType = "Para";
const ShapeType GeoPara::s_classTypeID = 0x12;
GeoPara::GeoPara (double XHalfLength, double YHalfLength, double ZHalfLength, double Alpha, double Theta, double Phi)
  :
m_xHalfLength (XHalfLength),
m_yHalfLength (YHalfLength),
m_zHalfLength (ZHalfLength),
m_theta (Theta),
m_alpha (Alpha),
m_phi (Phi)
{}

GeoPara::~GeoPara()
{}

double GeoPara::volume () const
{
  return 8.0 * m_xHalfLength * m_yHalfLength * m_zHalfLength;
}

const std::string & GeoPara::type () const
{
  return s_classType;
}

ShapeType GeoPara::typeID () const
{
  return s_classTypeID;
}

void GeoPara::exec (GeoShapeAction *action) const
{
	action->handlePara(this);
}

