/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoModelKernel/GeoAbsPositionInfo.h"
#include "GeoModelKernel/GeoDefinitions.h"
#include <memory>

GeoAbsPositionInfo::GeoAbsPositionInfo()
  : m_absTransform(nullptr),
    m_defAbsTransform(nullptr)
{
}

GeoAbsPositionInfo::~GeoAbsPositionInfo()
{
  delete m_absTransform;
  delete m_defAbsTransform;
}

void GeoAbsPositionInfo::clearAbsTransform ()
{
  delete m_absTransform;
  m_absTransform = nullptr;
}

void GeoAbsPositionInfo::clearDefAbsTransform ()
{
  delete m_defAbsTransform;
  m_defAbsTransform = nullptr;
}

void GeoAbsPositionInfo::setAbsTransform (const GeoTrf::Transform3D &  xform)
{
  if (m_absTransform)
    {
      (*m_absTransform) = xform;
    }
  else
    {
      m_absTransform = new GeoTrf::Transform3D (xform);
    }
}

void GeoAbsPositionInfo::setDefAbsTransform (const GeoTrf::Transform3D &  xform)
{
  if (m_defAbsTransform)
    {
      (*m_defAbsTransform) = xform;
    }
  else
    {
      m_defAbsTransform = new GeoTrf::Transform3D (xform);
    }
}
