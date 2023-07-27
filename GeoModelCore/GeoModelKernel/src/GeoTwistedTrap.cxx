/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "GeoModelKernel/GeoTwistedTrap.h"
#include "GeoModelKernel/GeoShapeAction.h"
#include <iostream>

const std::string GeoTwistedTrap::s_classType = "TwistedTrap";
const ShapeType GeoTwistedTrap::s_classTypeID = 0x19; //this code should not be used by other shapes
 

GeoTwistedTrap::GeoTwistedTrap(double  pPhiTwist,
                               double  pDx1,
                               double  pDx2,
                               double  pDy,
                               double  pDz )
: m_theta(0.),
m_phi(0.),
m_dy1(pDy),
m_dx1(pDx1),
m_dx2(pDx2),
m_dy2(pDy),
m_dx3(pDx1),
m_dx4(pDx2),
m_dz(pDz),
m_alph(0.),
m_phiTwist(pPhiTwist)
{
}

GeoTwistedTrap::
GeoTwistedTrap(double  pPhiTwist,  // twist angle
               double  pDz,        // half z length
               double  pTheta,  // direction between end planes
               double  pPhi,    // defined by polar and azimuthal angles
               double  pDy1,    // half y length at -pDz
               double  pDx1,    // half x length at -pDz,-pDy
               double  pDx2,    // half x length at -pDz,+pDy
               double  pDy2,    // half y length at +pDz
               double  pDx3,    // half x length at +pDz,-pDy
               double  pDx4,    // half x length at +pDz,+pDy
               double  pAlph )  // tilt angle
: m_theta(pTheta),
m_phi(pPhi),
m_dy1(pDy1),
m_dx1(pDx1),
m_dx2(pDx2),
m_dy2(pDy2),
m_dx3(pDx3),
m_dx4(pDx4),
m_dz(pDz),
m_alph(pAlph),
m_phiTwist(pPhiTwist)
{
     if  ( ! ( ( m_dx1  > 2*m_CarTolerance)
            && ( m_dx2  > 2*m_CarTolerance)
            && ( m_dx3  > 2*m_CarTolerance)
            && ( m_dx4  > 2*m_CarTolerance)
            && ( m_dy1  > 2*m_CarTolerance)
            && ( m_dy2  > 2*m_CarTolerance)
            && ( m_dz   > 2*m_CarTolerance)
            && ( std::fabs(m_phiTwist) > 2*m_AngTolerance )
            && ( std::fabs(m_phiTwist) < SYSTEM_OF_UNITS::pi/2 )
            && ( std::fabs(m_alph) < SYSTEM_OF_UNITS::pi/2 )
            && ( m_theta < SYSTEM_OF_UNITS::pi/2 && m_theta >= 0 ) )
         )
     {
      std::cout<< "EXCEPTION!!! Invalid dimensions. Too small, or twist angle too big: "<< std::endl
               << " fDx 1-4 = " << m_dx1/SYSTEM_OF_UNITS::cm << ", " << m_dx2/SYSTEM_OF_UNITS::cm << ", "
               << m_dx3/SYSTEM_OF_UNITS::cm << ", " << m_dx4/SYSTEM_OF_UNITS::cm << " cm" << std::endl
               << " fDy 1-2 = " << m_dy1/SYSTEM_OF_UNITS::cm << ", " << m_dy2/SYSTEM_OF_UNITS::cm << ", "
               << " cm" << std::endl
               << " fDz = " << m_dz/SYSTEM_OF_UNITS::cm << " cm" << std::endl
               << " twist angle " << m_phiTwist/SYSTEM_OF_UNITS::deg << " deg"<< std::endl
               << " phi,theta = " << m_phi/SYSTEM_OF_UNITS::deg << ", "  << m_theta/SYSTEM_OF_UNITS::deg << " deg"<< std::endl
               << " tilt angle alpha = " << m_alph/SYSTEM_OF_UNITS::deg << " deg"<< std::endl
               << " twistangle should be > "<< (2*m_AngTolerance)/SYSTEM_OF_UNITS::deg<<" and < "<< (SYSTEM_OF_UNITS::pi/2)/SYSTEM_OF_UNITS::deg << " deg"<<std::endl
               << " theta should be >= 0 and < "<< (SYSTEM_OF_UNITS::pi/2)/SYSTEM_OF_UNITS::deg << " deg"<< std::endl
               << " tilt angle alpha should be < "<< (SYSTEM_OF_UNITS::pi/2)/SYSTEM_OF_UNITS::deg << " deg"<<std::endl
               << std::endl;
    
     }
}


GeoTwistedTrap::~GeoTwistedTrap()
{
}

double GeoTwistedTrap::volume () const
{
  return m_dz * ((m_dx1 + m_dx2 + m_dx3 + m_dx4) * (m_dy1 + m_dy2) +
                 (m_dx4 + m_dx3 - m_dx2 - m_dx1) * (m_dy2 - m_dy1) * (1./3.));
}

const std::string & GeoTwistedTrap::type () const
{
  return s_classType;
}

ShapeType GeoTwistedTrap::typeID () const
{
  return s_classTypeID;
}

void GeoTwistedTrap::exec (GeoShapeAction *action) const
{
  action->handleTwistedTrap(this); 
}
