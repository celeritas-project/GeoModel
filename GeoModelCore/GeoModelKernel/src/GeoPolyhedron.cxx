
#include "GeoModelKernel/GeoPolyhedron.h"

#include <iostream>
#include <vector>

#define perMillion 0.000001
#define deg (M_PI/180.0)

#include <float.h>    //G.Barrand : to have DBL_EPSILON on Windows.

// G.Barrand : introduce iabs to avoid a mess with cmath and some compiler.
inline int
iabs (int a)
{
  return a < 0 ? -a : a;
}
inline float
ffabs (float a)
{
  return a < 0.0f ? -a : a;
}

//--------------------------------------------------------------------//
// JFB:                                                               //
// GeoPolyhedron was GeoPolyhedron, retrofitted to GeoModel           //
// infrastructure:                                                    //
//--------------------------------------------------------------------//

//
// ********************************************************************
// * DISCLAIMER                                                       *
// *                                                                  *
// * The following disclaimer summarizes all the specific disclaimers *
// * of contributors to this software. The specific disclaimers,which *
// * govern, are listed with their locations in:                      *
// *   http://cern.ch/geant4/license                                  *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.                                                             *
// *                                                                  *
// * This  code  implementation is the  intellectual property  of the *
// * GEANT4 collaboration.                                            *
// * By copying,  distributing  or modifying the Program (or any work *
// * based  on  the Program)  you indicate  your  acceptance of  this *
// * statement, and all its terms.                                    *
// ********************************************************************
//
//
//
//
// G4 Polyhedron library
//
// History:
// 23.07.96 E.Chernyaev <Evgueni.Tcherniaev@cern.ch> - initial version
//
// 30.09.96 E.Chernyaev
// - added GetNextVertexIndex, GetVertex by Yasuhide Sawada
// - added GetNextUnitNormal, GetNextEdgeIndeces, GetNextEdge
//
// 15.12.96 E.Chernyaev
// - added GetNumberOfRotationSteps, RotateEdge, RotateAroundZ, SetReferences
// - rewritten G4PolyhedronCons;
// - added G4PolyhedronPara, ...Trap, ...Pgon, ...Pcon, ...Sphere, ...Torus
//
// 01.06.97 E.Chernyaev
// - modified RotateAroundZ, added SetSideFacets
//
// 19.03.00 E.Chernyaev
// - implemented boolean operations (add, subtract, intersect) on polyhedra;
//
// 25.05.01 E.Chernyaev
// - added GetSurfaceArea() and GetVolume();
//


/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedron operator <<                   Date:    09.05.96 *
 * Author: E.Chernyaev (IHEP/Protvino)               Revised:          *
 *                                                                     *
 * Function: Print contents of G4 polyhedron                           *
 *                                                                     *
 ***********************************************************************/
std::ostream & operator<< (std::ostream & ostr, const GeoPolyhedron::GeoFacet & facet)
{
  for (int k = 0; k < 4; k++)
    {
      ostr << " " << facet.m_edge[k].v << "/" << facet.m_edge[k].f;
    }
  return ostr;
}

std::ostream & operator<< (std::ostream & ostr, const GeoPolyhedron & ph)
{
  ostr << std::endl;
  ostr << "Nverteces=" << ph.m_nvert << ", Nfacets=" << ph.m_nface << std::endl;
  int i;
  for (i = 1; i <= ph.m_nvert; i++)
    {
      ostr << "xyz(" << i << ")="
        << ph.m_pV[i][0] << ' ' << ph.m_pV[i][1] << ' ' << ph.m_pV[i][2]
  << std::endl;
    }
  for (i = 1; i <= ph.m_nface; i++)
    {
      ostr << "face(" << i << ")=" << ph.m_pF[i] << std::endl;
    }
  return ostr;
}

GeoPolyhedron::GeoPolyhedron (const GeoPolyhedron & from)
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedron copy constructor             Date:    23.07.96  *
 * Author: E.Chernyaev (IHEP/Protvino)              Revised:           *
 *                                                                     *
 ***********************************************************************/
{
  if (from.m_nvert > 0 && from.m_nface > 0)
    {
      m_nvert = from.m_nvert;
      m_nface = from.m_nface;
      m_pV = new GeoTrf::Vector3D[m_nvert + 1];
      m_pF = new GeoFacet[m_nface + 1];
      int i;
      for (i = 0; i <= m_nvert; i++)
  m_pV[i] = from.m_pV[i];
      for (i = 0; i <= m_nface; i++)
  m_pF[i] = from.m_pF[i];
    }
  else
    {
      m_nvert = 0;
      m_nface = 0;
      m_pV = 0;
      m_pF = 0;
    }
}

GeoPolyhedron & GeoPolyhedron::operator= (const GeoPolyhedron & from)
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedron operator =                   Date:    23.07.96  *
 * Author: E.Chernyaev (IHEP/Protvino)              Revised:           *
 *                                                                     *
 * Function: Copy contents of one GEANT4 polyhedron to another         *
 *                                                                     *
 ***********************************************************************/
{
  if (this == &from)
    return *this;
  delete[]m_pV;
  delete[]m_pF;
  if (from.m_nvert > 0 && from.m_nface > 0)
    {
      m_nvert = from.m_nvert;
      m_nface = from.m_nface;
      m_pV = new GeoTrf::Vector3D[m_nvert + 1];
      m_pF = new GeoFacet[m_nface + 1];
      int
  i;
      for (i = 1; i <= m_nvert; i++)
  m_pV[i] = from.m_pV[i];
      for (i = 1; i <= m_nface; i++)
  m_pF[i] = from.m_pF[i];
    }
  else
    {
      m_nvert = 0;
      m_nface = 0;
      m_pV = 0;
      m_pF = 0;
    }
  return *this;
}

int
GeoPolyhedron::FindNeighbour (int iFace, int iNode, int iOrder) const
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedron::FindNeighbour                Date:    22.11.99 *
 * Author: E.Chernyaev                               Revised:          *
 *                                                                     *
 * Function: Find neighbouring face                                    *
 *                                                                     *
 ***********************************************************************/
{
  int i;
  for (i = 0; i < 4; i++)
    {
      if (iNode == iabs (m_pF[iFace].m_edge[i].v))
  break;
    }
  if (i == 4)
    {
      std::cerr
  << "GeoPolyhedron::FindNeighbour: face " << iFace
  << " has no node " << iNode << std::endl;
      return 0;
    }
  if (iOrder < 0)
    {
      if (--i < 0)
  i = 3;
      if (m_pF[iFace].m_edge[i].v == 0)
  i = 2;
    }
  return (m_pF[iFace].m_edge[i].v > 0) ? 0 : m_pF[iFace].m_edge[i].f;
}

GeoTrf::Vector3D GeoPolyhedron::FindNodeNormal (int iFace, int iNode) const
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedron::FindNodeNormal               Date:    22.11.99 *
 * Author: E.Chernyaev                               Revised:          *
 *                                                                     *
 * Function: Find normal at given node                                 *
 *                                                                     *
 ***********************************************************************/
{
  GeoTrf::Vector3D
    normal =
    GetUnitNormal (iFace);
  int
    k =
    iFace,
    iOrder =
    1,
    n =
    1;

  for (;;)
    {
      k = FindNeighbour (k, iNode, iOrder);
      if (k == iFace)
  break;
      if (k > 0)
  {
    n++;
    normal += GetUnitNormal (k);
  }
      else
  {
    if (iOrder < 0)
      break;
    k = iFace;
    iOrder = -iOrder;
  }
    }
  return normal.normalized();
}

void
GeoPolyhedron::SetNumberOfRotationSteps (int n)
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedron::SetNumberOfRotationSteps     Date:    24.06.97 *
 * Author: J.Allison (Manchester University)         Revised:          *
 *                                                                     *
 * Function: Set number of steps for whole circle                      *
 *                                                                     *
 ***********************************************************************/
{
  const int nMin = 3;
  if (n < nMin)
    {
      std::cerr
  << "GeoPolyhedron::SetNumberOfRotationSteps: attempt to set the\n"
  << "number of steps per circle < " << nMin << "; forced to " << nMin
  << std::endl;
      s_fNumberOfRotationSteps = nMin;
    }
  else
    {
      s_fNumberOfRotationSteps = n;
    }
}

void
GeoPolyhedron::AllocateMemory (int Nvert, int Nface)
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedron::AllocateMemory               Date:    19.06.96 *
 * Author: E.Chernyaev (IHEP/Protvino)               Revised:          *
 *                                                                     *
 * Function: Allocate memory for GEANT4 polyhedron                     *
 *                                                                     *
 * Input: Nvert - number of nodes                                      *
 *        Nface - number of faces                                      *
 *                                                                     *
 ***********************************************************************/
{
  m_nvert = Nvert;
  m_nface = Nface;
  m_pV = new GeoTrf::Vector3D[m_nvert + 1];
  m_pF = new GeoFacet[m_nface + 1];
}

void
GeoPolyhedron::CreatePrism ()
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedron::CreatePrism                  Date:    15.07.96 *
 * Author: E.Chernyaev (IHEP/Protvino)               Revised:          *
 *                                                                     *
 * Function: Set facets for a prism                                    *
 *                                                                     *
 ***********************************************************************/
{
  enum
  { DUMMY, BOTTOM, LEFT, BACK, RIGHT, FRONT, TOP };

  m_pF[1] = GeoFacet (1, LEFT, 4, BACK, 3, RIGHT, 2, FRONT);
  m_pF[2] = GeoFacet (5, TOP, 8, BACK, 4, BOTTOM, 1, FRONT);
  m_pF[3] = GeoFacet (8, TOP, 7, RIGHT, 3, BOTTOM, 4, LEFT);
  m_pF[4] = GeoFacet (7, TOP, 6, FRONT, 2, BOTTOM, 3, BACK);
  m_pF[5] = GeoFacet (6, TOP, 5, LEFT, 1, BOTTOM, 2, RIGHT);
  m_pF[6] = GeoFacet (5, FRONT, 6, RIGHT, 7, BACK, 8, LEFT);
}

void
GeoPolyhedron::RotateEdge (int k1, int k2, double r1, double r2,
         int v1, int v2, int vEdge,
         bool ifWholeCircle, int ns, int &kface)
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedron::RotateEdge                   Date:    05.12.96 *
 * Author: E.Chernyaev (IHEP/Protvino)               Revised:          *
 *                                                                     *
 * Function: Create set of facets by rotation of an edge around Z-axis *
 *                                                                     *
 * Input: k1, k2 - end vertices of the edge                            *
 *        r1, r2 - radiuses of the end vertices                        *
 *        v1, v2 - visibility of edges produced by rotation of the end *
 *                 vertices                                            *
 *        vEdge  - visibility of the edge                              *
 *        ifWholeCircle - is true in case of whole circle rotation     *
 *        ns     - number of discrete steps                            *
 *        r[]    - r-coordinates                                       *
 *        kface  - current free cell in the m_pF array                 *
 *                                                                     *
 ***********************************************************************/
{
  if (r1 == 0. && r2 == 0)
    return;

  int i;
  int i1 = k1;
  int i2 = k2;
  int ii1 = ifWholeCircle ? i1 : i1 + ns;
  int ii2 = ifWholeCircle ? i2 : i2 + ns;
  int vv = ifWholeCircle ? vEdge : 1;

  if (ns == 1)
    {
      if (r1 == 0.)
  {
    m_pF[kface++] = GeoFacet (i1, 0, v2 * i2, 0, (i2 + 1), 0);
  }
      else if (r2 == 0.)
  {
    m_pF[kface++] = GeoFacet (i1, 0, i2, 0, v1 * (i1 + 1), 0);
  }
      else
  {
    m_pF[kface++] =
      GeoFacet (i1, 0, v2 * i2, 0, (i2 + 1), 0, v1 * (i1 + 1), 0);
  }
    }
  else
    {
      if (r1 == 0.)
  {
    m_pF[kface++] =
      GeoFacet (vv * i1, 0, v2 * i2, 0, vEdge * (i2 + 1), 0);
    for (i2++, i = 1; i < ns - 1; i2++, i++)
      {
        m_pF[kface++] =
    GeoFacet (vEdge * i1, 0, v2 * i2, 0, vEdge * (i2 + 1), 0);
      }
    m_pF[kface++] = GeoFacet (vEdge * i1, 0, v2 * i2, 0, vv * ii2, 0);
  }
      else if (r2 == 0.)
  {
    m_pF[kface++] =
      GeoFacet (vv * i1, 0, vEdge * i2, 0, v1 * (i1 + 1), 0);
    for (i1++, i = 1; i < ns - 1; i1++, i++)
      {
        m_pF[kface++] =
    GeoFacet (vEdge * i1, 0, vEdge * i2, 0, v1 * (i1 + 1), 0);
      }
    m_pF[kface++] = GeoFacet (vEdge * i1, 0, vv * i2, 0, v1 * ii1, 0);
  }
      else
  {
    m_pF[kface++] =
      GeoFacet (vv * i1, 0, v2 * i2, 0, vEdge * (i2 + 1), 0,
          v1 * (i1 + 1), 0);
    for (i1++, i2++, i = 1; i < ns - 1; i1++, i2++, i++)
      {
        m_pF[kface++] =
    GeoFacet (vEdge * i1, 0, v2 * i2, 0, vEdge * (i2 + 1), 0,
        v1 * (i1 + 1), 0);
      }
    m_pF[kface++] =
      GeoFacet (vEdge * i1, 0, v2 * i2, 0, vv * ii2, 0, v1 * ii1, 0);
  }
    }
}

void
GeoPolyhedron::SetSideFacets (int ii[4], int vv[4],
            int *kk, double *r,
            double dphi, int ns, int &kface)
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedron::SetSideFacets                Date:    20.05.97 *
 * Author: E.Chernyaev (IHEP/Protvino)               Revised:          *
 *                                                                     *
 * Function: Set side facets for the case of incomplete rotation       *
 *                                                                     *
 * Input: ii[4] - indeces of original verteces                         *
 *        vv[4] - visibility of edges                                  *
 *        kk[]  - indeces of nodes                                     *
 *        r[]   - radiuses                                             *
 *        dphi  - delta phi                                            *
 *        ns     - number of discrete steps                            *
 *        kface  - current free cell in the pF array                   *
 *                                                                     *
 ***********************************************************************/
{
  int k1, k2, k3, k4;
  if (fabs (dphi - M_PI) < perMillion)
    {        // half a circle
      for (int i = 0; i < 4; i++)
  {
    k1 = ii[i];
    k2 = (i == 3) ? ii[0] : ii[i + 1];
    if (r[k1] == 0. && r[k2] == 0.)
      vv[i] = -1;
  }
    }

  if (ii[1] == ii[2])
    {
      k1 = kk[ii[0]];
      k2 = kk[ii[2]];
      k3 = kk[ii[3]];
      m_pF[kface++] = GeoFacet (vv[0] * k1, 0, vv[2] * k2, 0, vv[3] * k3, 0);
      if (r[ii[0]] != 0.)
  k1 += ns;
      if (r[ii[2]] != 0.)
  k2 += ns;
      if (r[ii[3]] != 0.)
  k3 += ns;
      m_pF[kface++] = GeoFacet (vv[2] * k3, 0, vv[0] * k2, 0, vv[3] * k1, 0);
    }
  else if (kk[ii[0]] == kk[ii[1]])
    {
      k1 = kk[ii[0]];
      k2 = kk[ii[2]];
      k3 = kk[ii[3]];
      m_pF[kface++] = GeoFacet (vv[1] * k1, 0, vv[2] * k2, 0, vv[3] * k3, 0);
      if (r[ii[0]] != 0.)
  k1 += ns;
      if (r[ii[2]] != 0.)
  k2 += ns;
      if (r[ii[3]] != 0.)
  k3 += ns;
      m_pF[kface++] = GeoFacet (vv[2] * k3, 0, vv[1] * k2, 0, vv[3] * k1, 0);
    }
  else if (kk[ii[2]] == kk[ii[3]])
    {
      k1 = kk[ii[0]];
      k2 = kk[ii[1]];
      k3 = kk[ii[2]];
      m_pF[kface++] = GeoFacet (vv[0] * k1, 0, vv[1] * k2, 0, vv[3] * k3, 0);
      if (r[ii[0]] != 0.)
  k1 += ns;
      if (r[ii[1]] != 0.)
  k2 += ns;
      if (r[ii[2]] != 0.)
  k3 += ns;
      m_pF[kface++] = GeoFacet (vv[1] * k3, 0, vv[0] * k2, 0, vv[3] * k1, 0);
    }
  else
    {
      k1 = kk[ii[0]];
      k2 = kk[ii[1]];
      k3 = kk[ii[2]];
      k4 = kk[ii[3]];
      m_pF[kface++] =
  GeoFacet (vv[0] * k1, 0, vv[1] * k2, 0, vv[2] * k3, 0, vv[3] * k4, 0);
      if (r[ii[0]] != 0.)
  k1 += ns;
      if (r[ii[1]] != 0.)
  k2 += ns;
      if (r[ii[2]] != 0.)
  k3 += ns;
      if (r[ii[3]] != 0.)
  k4 += ns;
      m_pF[kface++] =
  GeoFacet (vv[2] * k4, 0, vv[1] * k3, 0, vv[0] * k2, 0, vv[3] * k1, 0);
    }
}

void
GeoPolyhedron::RotateAroundZ (int nstep, double phi, double dphi,
            int np1, int np2,
            const double *z, double *r,
            int nodeVis, int edgeVis)
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedron::RotateAroundZ                Date:    27.11.96 *
 * Author: E.Chernyaev (IHEP/Protvino)               Revised:          *
 *                                                                     *
 * Function: Create GeoPolyhedron for a solid produced by rotation of  *
 *           two polylines around Z-axis                               *
 *                                                                     *
 * Input: nstep - number of discrete steps, if 0 then default          *
 *        phi   - starting phi angle                                   *
 *        dphi  - delta phi                                            *
 *        np1   - number of points in external polyline                *
 *                (must be negative in case of closed polyline)        *
 *        np2   - number of points in internal polyline (may be 1)     *
 *        z[]   - z-coordinates (+z >>> -z for both polylines)         *
 *        r[]   - r-coordinates                                        *
 *        nodeVis - how to Draw edges joing consecutive positions of   *
 *                  node during rotation                               *
 *        edgeVis - how to Draw edges                                  *
 *                                                                     *
 ***********************************************************************/
{
  static double wholeCircle = 2 * M_PI;

  //   S E T   R O T A T I O N   P A R A M E T E R S

  bool ifWholeCircle = (fabs (dphi - wholeCircle) < perMillion) ?
    true : false;
  double delPhi = ifWholeCircle ? wholeCircle : dphi;
  int nSphi = (nstep > 0) ?
    nstep : int (delPhi * GetNumberOfRotationSteps () / wholeCircle + .5);
  if (nSphi == 0)
    nSphi = 1;
  int nVphi = ifWholeCircle ? nSphi : nSphi + 1;
  bool ifClosed = np1 > 0 ? false : true;

  //   C O U N T   V E R T E C E S

  int absNp1 = iabs (np1);
  int absNp2 = iabs (np2);
  int i1beg = 0;
  int i1end = absNp1 - 1;
  int i2beg = absNp1;
  int i2end = absNp1 + absNp2 - 1;
  int i, j, k;

  for (i = i1beg; i <= i2end; i++)
    {
      if (fabs (r[i]) < perMillion)
  r[i] = 0.;
    }

  j = 0;      // external nodes
  for (i = i1beg; i <= i1end; i++)
    {
      j += (r[i] == 0.) ? 1 : nVphi;
    }

  bool ifSide1 = false;    // internal nodes
  bool ifSide2 = false;

  if (r[i2beg] != r[i1beg] || z[i2beg] != z[i1beg])
    {
      j += (r[i2beg] == 0.) ? 1 : nVphi;
      ifSide1 = true;
    }

  for (i = i2beg + 1; i < i2end; i++)
    {
      j += (r[i] == 0.) ? 1 : nVphi;
    }

  if (r[i2end] != r[i1end] || z[i2end] != z[i1end])
    {
      if (absNp2 > 1)
  j += (r[i2end] == 0.) ? 1 : nVphi;
      ifSide2 = true;
    }

  //   C O U N T   F A C E S

  k = ifClosed ? absNp1 * nSphi : (absNp1 - 1) * nSphi;  // external faces

  if (absNp2 > 1)
    {        // internal faces
      for (i = i2beg; i < i2end; i++)
  {
    if (r[i] > 0. || r[i + 1] > 0.)
      k += nSphi;
  }

      if (ifClosed)
  {
    if (r[i2end] > 0. || r[i2beg] > 0.)
      k += nSphi;
  }
    }

  if (!ifClosed)
    {        // side faces
      if (ifSide1 && (r[i1beg] > 0. || r[i2beg] > 0.))
  k += nSphi;
      if (ifSide2 && (r[i1end] > 0. || r[i2end] > 0.))
  k += nSphi;
    }

  if (!ifWholeCircle)
    {        // phi_side faces
      k += ifClosed ? 2 * absNp1 : 2 * (absNp1 - 1);
    }

  //   A L L O C A T E   M E M O R Y

  AllocateMemory (j, k);

  //   G E N E R A T E   V E R T E C E S

  int *kk;
  kk = new int[absNp1 + absNp2];

  k = 1;
  for (i = i1beg; i <= i1end; i++)
    {
      kk[i] = k;
      if (r[i] == 0.)
  {
    m_pV[k++] = GeoTrf::Vector3D (0, 0, z[i]);
  }
      else
  {
    k += nVphi;
  }
    }

  i = i2beg;
  if (ifSide1)
    {
      kk[i] = k;
      if (r[i] == 0.)
  {
    m_pV[k++] = GeoTrf::Vector3D (0, 0, z[i]);
  }
      else
  {
    k += nVphi;
  }
    }
  else
    {
      kk[i] = kk[i1beg];
    }

  for (i = i2beg + 1; i < i2end; i++)
    {
      kk[i] = k;
      if (r[i] == 0.)
  {
    m_pV[k++] = GeoTrf::Vector3D (0, 0, z[i]);
  }
      else
  {
    k += nVphi;
  }
    }

  if (absNp2 > 1)
    {
      i = i2end;
      if (ifSide2)
  {
    kk[i] = k;
    if (r[i] == 0.)
      m_pV[k] = GeoTrf::Vector3D (0, 0, z[i]);
  }
      else
  {
    kk[i] = kk[i1end];
  }
    }

  double cosPhi, sinPhi;

  double fac = delPhi / nSphi;
  for (j = 0; j < nVphi; j++)
    {
      cosPhi = cos (phi + j * fac);
      sinPhi = sin (phi + j * fac);
      for (i = i1beg; i <= i2end; i++)
  {
    if (r[i] != 0.)
      m_pV[kk[i] + j] = GeoTrf::Vector3D (r[i] * cosPhi, r[i] * sinPhi, z[i]);
  }
    }

  //   G E N E R A T E   E X T E R N A L   F A C E S

  int v1, v2;

  k = 1;
  v2 = ifClosed ? nodeVis : 1;
  for (i = i1beg; i < i1end; i++)
    {
      v1 = v2;
      if (!ifClosed && i == i1end - 1)
  {
    v2 = 1;
  }
      else
  {
    v2 = (r[i] == r[i + 1] && r[i + 1] == r[i + 2]) ? -1 : nodeVis;
  }
      RotateEdge (kk[i], kk[i + 1], r[i], r[i + 1], v1, v2,
      edgeVis, ifWholeCircle, nSphi, k);
    }
  if (ifClosed)
    {
      RotateEdge (kk[i1end], kk[i1beg], r[i1end], r[i1beg], nodeVis, nodeVis,
      edgeVis, ifWholeCircle, nSphi, k);
    }

  //   G E N E R A T E   I N T E R N A L   F A C E S

  if (absNp2 > 1)
    {
      v2 = ifClosed ? nodeVis : 1;
      for (i = i2beg; i < i2end; i++)
  {
    v1 = v2;
    if (!ifClosed && i == i2end - 1)
      {
        v2 = 1;
      }
    else
      {
        v2 = (r[i] == r[i + 1] && r[i + 1] == r[i + 2]) ? -1 : nodeVis;
      }
    RotateEdge (kk[i + 1], kk[i], r[i + 1], r[i], v2, v1,
          edgeVis, ifWholeCircle, nSphi, k);
  }
      if (ifClosed)
  {
    RotateEdge (kk[i2beg], kk[i2end], r[i2beg], r[i2end], nodeVis,
          nodeVis, edgeVis, ifWholeCircle, nSphi, k);
  }
    }

  //   G E N E R A T E   S I D E   F A C E S

  if (!ifClosed)
    {
      if (ifSide1)
  {
    RotateEdge (kk[i2beg], kk[i1beg], r[i2beg], r[i1beg], 1, 1,
          -1, ifWholeCircle, nSphi, k);
  }
      if (ifSide2)
  {
    RotateEdge (kk[i1end], kk[i2end], r[i1end], r[i2end], 1, 1,
          -1, ifWholeCircle, nSphi, k);
  }
    }

  //   G E N E R A T E   S I D E   F A C E S  for the case of incomplete circle

  if (!ifWholeCircle)
    {

      int ii[4], vv[4];

      if (ifClosed)
  {
    for (i = i1beg; i <= i1end; i++)
      {
        ii[0] = i;
        ii[3] = (i == i1end) ? i1beg : i + 1;
        ii[1] = (absNp2 == 1) ? i2beg : ii[0] + absNp1;
        ii[2] = (absNp2 == 1) ? i2beg : ii[3] + absNp1;
        vv[0] = -1;
        vv[1] = 1;
        vv[2] = -1;
        vv[3] = 1;
        SetSideFacets (ii, vv, kk, r, dphi, nSphi, k);
      }
  }
      else
  {
    for (i = i1beg; i < i1end; i++)
      {
        ii[0] = i;
        ii[3] = i + 1;
        ii[1] = (absNp2 == 1) ? i2beg : ii[0] + absNp1;
        ii[2] = (absNp2 == 1) ? i2beg : ii[3] + absNp1;
        vv[0] = (i == i1beg) ? 1 : -1;
        vv[1] = 1;
        vv[2] = (i == i1end - 1) ? 1 : -1;
        vv[3] = 1;
        SetSideFacets (ii, vv, kk, r, dphi, nSphi, k);
      }
  }
    }

  delete[]kk;

  if (k - 1 != m_nface)
    {
      std::cerr
  << "Polyhedron::RotateAroundZ: number of generated faces ("
  << k - 1 << ") is not equal to the number of allocated faces ("
  << m_nface << ")" << std::endl;
    }
}

void
GeoPolyhedron::SetReferences ()
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedron::SetReferences                Date:    04.12.96 *
 * Author: E.Chernyaev (IHEP/Protvino)               Revised:          *
 *                                                                     *
 * Function: For each edge set reference to neighbouring facet         *
 *                                                                     *
 ***********************************************************************/
{
  if (m_nface <= 0)
    return;

  struct edgeListMember
  {
    edgeListMember *next;
    int v2;
    int iface;
    int iedge;
  }
   *edgeList, *freeList, **headList;


  //   A L L O C A T E   A N D   I N I T I A T E   L I S T S

  edgeList = new edgeListMember[2 * m_nface];
  headList = new edgeListMember *[m_nvert];

  int i;
  for (i = 0; i < m_nvert; i++)
    {
      headList[i] = 0;
    }
  freeList = edgeList;
  for (i = 0; i < 2 * m_nface - 1; i++)
    {
      edgeList[i].next = &edgeList[i + 1];
    }
  edgeList[2 * m_nface - 1].next = 0;

  //   L O O P   A L O N G   E D G E S

  int iface, iedge, nedge, i1, i2, k1, k2;
  edgeListMember *prev, *cur;

  for (iface = 1; iface <= m_nface; iface++)
    {
      nedge = (m_pF[iface].m_edge[3].v == 0) ? 3 : 4;
      for (iedge = 0; iedge < nedge; iedge++)
  {
    i1 = iedge;
    i2 = (iedge < nedge - 1) ? iedge + 1 : 0;
    i1 = iabs (m_pF[iface].m_edge[i1].v);
    i2 = iabs (m_pF[iface].m_edge[i2].v);
    k1 = (i1 < i2) ? i1 : i2;  // k1 = ::min(i1,i2);
    k2 = (i1 > i2) ? i1 : i2;  // k2 = ::max(i1,i2);

    // check head of the List corresponding to k1
    cur = headList[k1];
    if (cur == 0)
      {
        headList[k1] = freeList;
        freeList = freeList->next;
        cur = headList[k1];
        cur->next = 0;
        cur->v2 = k2;
        cur->iface = iface;
        cur->iedge = iedge;
        continue;
      }

    if (cur->v2 == k2)
      {
        headList[k1] = cur->next;
        cur->next = freeList;
        freeList = cur;
        m_pF[iface].m_edge[iedge].f = cur->iface;
        m_pF[cur->iface].m_edge[cur->iedge].f = iface;
        i1 = (m_pF[iface].m_edge[iedge].v < 0) ? -1 : 1;
        i2 = (m_pF[cur->iface].m_edge[cur->iedge].v < 0) ? -1 : 1;
        if (i1 != i2)
    {
      std::cerr
        << "Polyhedron::SetReferences: different edge visibility "
        << iface << "/" << iedge << "/"
        << m_pF[iface].m_edge[iedge].v << " and "
        << cur->iface << "/" << cur->iedge << "/"
        << m_pF[cur->iface].m_edge[cur->iedge].v << std::endl;
    }
        continue;
      }

    // check List itself
    for (;;)
      {
        prev = cur;
        cur = prev->next;
        if (cur == 0)
    {
      prev->next = freeList;
      freeList = freeList->next;
      cur = prev->next;
      cur->next = 0;
      cur->v2 = k2;
      cur->iface = iface;
      cur->iedge = iedge;
      break;
    }

        if (cur->v2 == k2)
    {
      prev->next = cur->next;
      cur->next = freeList;
      freeList = cur;
      m_pF[iface].m_edge[iedge].f = cur->iface;
      m_pF[cur->iface].m_edge[cur->iedge].f = iface;
      i1 = (m_pF[iface].m_edge[iedge].v < 0) ? -1 : 1;
      i2 = (m_pF[cur->iface].m_edge[cur->iedge].v < 0) ? -1 : 1;
      if (i1 != i2)
        {
          std::cerr
      <<
      "Polyhedron::SetReferences: different edge visibility "
      << iface << "/" << iedge << "/" << m_pF[iface].
      m_edge[iedge].v << " and " << cur->iface << "/" << cur->
      iedge << "/" << m_pF[cur->iface].m_edge[cur->iedge].
      v << std::endl;
        }
      break;
    }
      }
  }
    }

  //  C H E C K   T H A T   A L L   L I S T S   A R E   E M P T Y

  for (i = 0; i < m_nvert; i++)
    {
      if (headList[i] != 0)
  {
    std::cerr
      << "Polyhedron::SetReferences: List " << i << " is not empty"
      << std::endl;
  }
    }

  //   F R E E   M E M O R Y

  delete[]edgeList;
  delete[]headList;
}

void
GeoPolyhedron::InvertFacets ()
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedron::InvertFacets                Date:    01.12.99  *
 * Author: E.Chernyaev                              Revised:           *
 *                                                                     *
 * Function: Invert the order of the nodes in the facets               *
 *                                                                     *
 ***********************************************************************/
{
  if (m_nface <= 0)
    return;
  int i, k, nnode, v[4]={0}, f[4]={0};
  for (i = 1; i <= m_nface; i++)
    {
      nnode = (m_pF[i].m_edge[3].v == 0) ? 3 : 4;
      for (k = 0; k < nnode; k++)
  {
    v[k] = (k + 1 == nnode) ? m_pF[i].m_edge[0].v : m_pF[i].m_edge[k + 1].v;
    if (v[k] * m_pF[i].m_edge[k].v < 0)
      v[k] = -v[k];
    f[k] = m_pF[i].m_edge[k].f;
  }
      for (k = 0; k < nnode; k++)
  {
    m_pF[i].m_edge[nnode - 1 - k].v = v[k];
    m_pF[i].m_edge[nnode - 1 - k].f = f[k];
  }
    }
}

GeoPolyhedron & GeoPolyhedron::Transform (const GeoTrf::RotationMatrix3D& rotation,
					  const GeoTrf::Vector3D & translation)
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedron::Transform                    Date:    01.12.99  *
 * Author: E.Chernyaev                              Revised:           *
 *                                                                     *
 * Function: Make transformation of the polyhedron                     *
 *                                                                     *
 ***********************************************************************/
{
  if (m_nvert > 0)
    {
      for (int i = 1; i <= m_nvert; i++)
  {
    m_pV[i] = rotation * m_pV[i] + translation;
  }

      //  C H E C K   D E T E R M I N A N T   A N D
      //  I N V E R T   F A C E T S   I F   I T   I S   N E G A T I V E

      GeoTrf::Vector3D x = rotation * GeoTrf::Vector3D (1, 0, 0);
      GeoTrf::Vector3D y = rotation * GeoTrf::Vector3D (0, 1, 0);
      GeoTrf::Vector3D z = rotation * GeoTrf::Vector3D (0, 0, 1);
      if ((x.cross (y)).dot (z) < 0)
  InvertFacets ();
    }
  return *this;
}

bool GeoPolyhedron::GetNextVertexIndex (int &index, int &edgeFlag) const
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedron::GetNextVertexIndex          Date:    03.09.96  *
 * Author: Yasuhide Sawada                          Revised:           *
 *                                                                     *
 * Function:                                                           *
 *                                                                     *
 ***********************************************************************/
{
  static int
    iFace =
    1;
  static int
    iQVertex =
    0;
  int
    vIndex =
    m_pF[iFace].
    m_edge[iQVertex].
    v;

  edgeFlag = (vIndex > 0) ? 1 : 0;
  index = iabs (vIndex);

  if (iQVertex >= 3 || m_pF[iFace].m_edge[iQVertex + 1].v == 0)
    {
      iQVertex = 0;
      if (++iFace > m_nface)
  iFace = 1;
      return false;    // Last Edge
    }
  else
    {
      ++iQVertex;
      return true;    // not Last Edge
    }
}

GeoTrf::Vector3D GeoPolyhedron::GetVertex (int index) const
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedron::GetVertex                   Date:    03.09.96  *
 * Author: Yasuhide Sawada                          Revised: 17.11.99  *
 *                                                                     *
 * Function: Get vertex of the index.                                  *
 *                                                                     *
 ***********************************************************************/
{
  if (index <= 0 || index > m_nvert)
    {
      std::cerr
  << "GeoPolyhedron::GetVertex: irrelevant index " << index
  << std::endl;
      return GeoTrf::Vector3D ();
    }
  return m_pV[index];
}

bool
GeoPolyhedron::GetNextVertex (GeoTrf::Vector3D & vertex, int &edgeFlag) const
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedron::GetNextVertex               Date:    22.07.96  *
 * Author: John Allison                             Revised:           *
 *                                                                     *
 * Function: Get vertices of the quadrilaterals in order for each      *
 *           face in face order.  Returns false when finished each     *
 *           face.                                                     *
 *                                                                     *
 ***********************************************************************/
{
  int index;
  bool rep = GetNextVertexIndex (index, edgeFlag);
  vertex = m_pV[index];
  return rep;
}

bool
  GeoPolyhedron::GetNextVertex (GeoTrf::Vector3D & vertex, int &edgeFlag,
        GeoTrf::Vector3D & normal) const
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedron::GetNextVertex               Date:    26.11.99  *
 * Author: E.Chernyaev                              Revised:           *
 *                                                                     *
 * Function: Get vertices with normals of the quadrilaterals in order  *
 *           for each face in face order.                              *
 *           Returns false when finished each face.                    *
 *                                                                     *
 ***********************************************************************/
{
  static int iFace = 1;
  static int iNode = 0;

  if (m_nface == 0)
    return false;    // empty polyhedron

  int k = m_pF[iFace].m_edge[iNode].v;
  if (k > 0)
    {
      edgeFlag = 1;
    }
  else
    {
      edgeFlag = -1;
      k = -k;
    }
  vertex = m_pV[k];
  normal = FindNodeNormal (iFace, k);
  if (iNode >= 3 || m_pF[iFace].m_edge[iNode + 1].v == 0)
    {
      iNode = 0;
      if (++iFace > m_nface)
  iFace = 1;
      return false;    // last node
    }
  else
    {
      ++iNode;
      return true;    // not last node
    }
}

bool
  GeoPolyhedron::GetNextEdgeIndeces (int &i1, int &i2, int &edgeFlag,
             int &iface1, int &iface2) const
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedron::GetNextEdgeIndeces          Date:    30.09.96  *
 * Author: E.Chernyaev                              Revised: 17.11.99  *
 *                                                                     *
 * Function: Get indeces of the next edge together with indeces of     *
 *           of the faces which share the edge.                        *
 *           Returns false when the last edge.                         *
 *                                                                     *
 ***********************************************************************/
{
  static int iFace = 1;
  static int iQVertex = 0;
  static int iOrder = 1;
  int k1, k2, kflag, kface1, kface2;

  if (iFace == 1 && iQVertex == 0)
    {
      k2 = m_pF[m_nface].m_edge[0].v;
      k1 = m_pF[m_nface].m_edge[3].v;
      if (k1 == 0)
  k1 = m_pF[m_nface].m_edge[2].v;
      if (iabs (k1) > iabs (k2))
  iOrder = -1;
    }

  do
    {
      k1 = m_pF[iFace].m_edge[iQVertex].v;
      kflag = k1;
      k1 = iabs (k1);
      kface1 = iFace;
      kface2 = m_pF[iFace].m_edge[iQVertex].f;
      if (iQVertex >= 3 || m_pF[iFace].m_edge[iQVertex + 1].v == 0)
  {
    iQVertex = 0;
    k2 = iabs (m_pF[iFace].m_edge[iQVertex].v);
    iFace++;
  }
      else
  {
    iQVertex++;
    k2 = iabs (m_pF[iFace].m_edge[iQVertex].v);
  }
    }
  while (iOrder * k1 > iOrder * k2);

  i1 = k1;
  i2 = k2;
  edgeFlag = (kflag > 0) ? 1 : 0;
  iface1 = kface1;
  iface2 = kface2;

  if (iFace > m_nface)
    {
      iFace = 1;
      iOrder = 1;
      return false;
    }
  else
    {
      return true;
    }
}

bool
GeoPolyhedron::GetNextEdgeIndeces (int &i1, int &i2, int &edgeFlag) const
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedron::GetNextEdgeIndeces          Date:    17.11.99  *
 * Author: E.Chernyaev                              Revised:           *
 *                                                                     *
 * Function: Get indeces of the next edge.                             *
 *           Returns false when the last edge.                         *
 *                                                                     *
 ***********************************************************************/
{
  int kface1, kface2;
  return GetNextEdgeIndeces (i1, i2, edgeFlag, kface1, kface2);
}

bool
  GeoPolyhedron::GetNextEdge (GeoTrf::Vector3D & p1,
            GeoTrf::Vector3D & p2, int &edgeFlag) const
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedron::GetNextEdge                 Date:    30.09.96  *
 * Author: E.Chernyaev                              Revised:           *
 *                                                                     *
 * Function: Get next edge.                                            *
 *           Returns false when the last edge.                         *
 *                                                                     *
 ***********************************************************************/
{
  int i1, i2;
  bool rep = GetNextEdgeIndeces (i1, i2, edgeFlag);
  p1 = m_pV[i1];
  p2 = m_pV[i2];
  return rep;
}

bool
  GeoPolyhedron::GetNextEdge (GeoTrf::Vector3D & p1, GeoTrf::Vector3D & p2,
            int &edgeFlag, int &iface1, int &iface2) const
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedron::GetNextEdge                 Date:    17.11.99  *
 * Author: E.Chernyaev                              Revised:           *
 *                                                                     *
 * Function: Get next edge with indeces of the faces which share       *
 *           the edge.                                                 *
 *           Returns false when the last edge.                         *
 *                                                                     *
 ***********************************************************************/
{
  int i1, i2;
  bool rep = GetNextEdgeIndeces (i1, i2, edgeFlag, iface1, iface2);
  p1 = m_pV[i1];
  p2 = m_pV[i2];
  return rep;
}

void
GeoPolyhedron::GetFacet (int iFace, int &n, int *iNodes,
       int *edgeFlags, int *iFaces) const
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedron::GetFacet                    Date:    15.12.99  *
 * Author: E.Chernyaev                              Revised:           *
 *                                                                     *
 * Function: Get face by index                                         *
 *                                                                     *
 ***********************************************************************/
{
  if (iFace < 1 || iFace > m_nface)
    {
      std::cerr
  << "GeoPolyhedron::GetFacet: irrelevant index " << iFace << std::endl;
      n = 0;
    }
  else
    {
      int i, k;
      for (i = 0; i < 4; i++)
  {
    k = m_pF[iFace].m_edge[i].v;
    if (k == 0)
      break;
    if (iFaces != 0)
      iFaces[i] = m_pF[iFace].m_edge[i].f;
    if (k > 0)
      {
        iNodes[i] = k;
        if (edgeFlags != 0)
    edgeFlags[i] = 1;
      }
    else
      {
        iNodes[i] = -k;
        if (edgeFlags != 0)
    edgeFlags[i] = -1;
      }
  }
      n = i;
    }
}

void
GeoPolyhedron::GetFacet (int index, int &n, GeoTrf::Vector3D * nodes,
       int *edgeFlags, GeoTrf::Vector3D * normals) const
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedron::GetFacet                    Date:    17.11.99  *
 * Author: E.Chernyaev                              Revised:           *
 *                                                                     *
 * Function: Get face by index                                         *
 *                                                                     *
 ***********************************************************************/
{
  int iNodes[4];
  GetFacet (index, n, iNodes, edgeFlags);
  if (n != 0)
    {
      for (int i = 0; i < 4; i++)
  {
    nodes[i] = m_pV[iNodes[i]];
    if (normals != 0)
      normals[i] = FindNodeNormal (index, iNodes[i]);
  }
    }
}

bool
  GeoPolyhedron::GetNextFacet (int &n, GeoTrf::Vector3D * nodes,
             int *edgeFlags, GeoTrf::Vector3D * normals) const
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedron::GetNextFacet                Date:    19.11.99  *
 * Author: E.Chernyaev                              Revised:           *
 *                                                                     *
 * Function: Get next face with normals of unit length at the nodes.   *
 *           Returns false when finished all faces.                    *
 *                                                                     *
 ***********************************************************************/
{
  static int iFace = 1;

  if (edgeFlags == 0)
    {
      GetFacet (iFace, n, nodes);
    }
  else if (normals == 0)
    {
      GetFacet (iFace, n, nodes, edgeFlags);
    }
  else
    {
      GetFacet (iFace, n, nodes, edgeFlags, normals);
    }

  if (++iFace > m_nface)
    {
      iFace = 1;
      return false;
    }
  else
    {
      return true;
    }
}

GeoTrf::Vector3D GeoPolyhedron::GetNormal (int iFace) const
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedron::GetNormal                    Date:    19.11.99 *
 * Author: E.Chernyaev                               Revised:          *
 *                                                                     *
 * Function: Get normal of the face given by index                     *
 *                                                                     *
 ***********************************************************************/
{
  if (iFace < 1 || iFace > m_nface)
    {
      std::cerr
  << "GeoPolyhedron::GetNormal: irrelevant index " << iFace
  << std::endl;
      return GeoTrf::Vector3D ();
    }

  int
    i0 =
    iabs (m_pF[iFace].m_edge[0].v);
  int
    i1 =
    iabs (m_pF[iFace].m_edge[1].v);
  int
    i2 =
    iabs (m_pF[iFace].m_edge[2].v);
  int
    i3 =
    iabs (m_pF[iFace].m_edge[3].v);
  if (i3 == 0)
    i3 = i0;
  return (m_pV[i2] - m_pV[i0]).cross (m_pV[i3] - m_pV[i1]);
}

GeoTrf::Vector3D GeoPolyhedron::GetUnitNormal (int iFace) const
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedron::GetNormal                    Date:    19.11.99 *
 * Author: E.Chernyaev                               Revised:          *
 *                                                                     *
 * Function: Get unit normal of the face given by index                *
 *                                                                     *
 ***********************************************************************/
{
  if (iFace < 1 || iFace > m_nface)
    {
      std::cerr
  << "GeoPolyhedron::GetUnitNormal: irrelevant index " << iFace
  << std::endl;
      return GeoTrf::Vector3D ();
    }

  int
    i0 =
    iabs (m_pF[iFace].m_edge[0].v);
  int
    i1 =
    iabs (m_pF[iFace].m_edge[1].v);
  int
    i2 =
    iabs (m_pF[iFace].m_edge[2].v);
  int
    i3 =
    iabs (m_pF[iFace].m_edge[3].v);
  if (i3 == 0)
    i3 = i0;
  GeoTrf::Vector3D
    nm = (m_pV[i2] - m_pV[i0]).
    cross (m_pV[i3] - m_pV[i1]);
  return nm.normalized();
}

bool GeoPolyhedron::GetNextNormal (GeoTrf::Vector3D & normal) const
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedron::GetNextNormal               Date:    22.07.96  *
 * Author: John Allison                             Revised: 19.11.99  *
 *                                                                     *
 * Function: Get normals of each face in face order.  Returns false    *
 *           when finished all faces.                                  *
 *                                                                     *
 ***********************************************************************/
{
  static int
    iFace =
    1;
  normal = GetNormal (iFace);
  if (++iFace > m_nface)
    {
      iFace = 1;
      return false;
    }
  else
    {
      return true;
    }
}

bool GeoPolyhedron::GetNextUnitNormal (GeoTrf::Vector3D & normal) const
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedron::GetNextUnitNormal           Date:    16.09.96  *
 * Author: E.Chernyaev                              Revised:           *
 *                                                                     *
 * Function: Get normals of unit length of each face in face order.    *
 *           Returns false when finished all faces.                    *
 *                                                                     *
 ***********************************************************************/
{
  bool
    rep =
    GetNextNormal (normal);
  normal = normal.normalized();
  return rep;
}

double
GeoPolyhedron::GetSurfaceArea () const
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedron::GetSurfaceArea              Date:    25.05.01  *
 * Author: E.Chernyaev                              Revised:           *
 *                                                                     *
 * Function: Returns area of the surface of the polyhedron.            *
 *                                                                     *
 ***********************************************************************/
{
  double s = 0.;
  for (int iFace = 1; iFace <= m_nface; iFace++)
    {
      int i0 = iabs (m_pF[iFace].m_edge[0].v);
      int i1 = iabs (m_pF[iFace].m_edge[1].v);
      int i2 = iabs (m_pF[iFace].m_edge[2].v);
      int i3 = iabs (m_pF[iFace].m_edge[3].v);
      if (i3 == 0)
  i3 = i0;
      s += ((m_pV[i2] - m_pV[i0]).cross (m_pV[i3] - m_pV[i1])).norm();
    }
  return s / 2.;
}

double
GeoPolyhedron::GetVolume () const
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedron::GetVolume                   Date:    25.05.01  *
 * Author: E.Chernyaev                              Revised:           *
 *                                                                     *
 * Function: Returns volume of the polyhedron.                         *
 *                                                                     *
 ***********************************************************************/
{
  double v = 0.;
  for (int iFace = 1; iFace <= m_nface; iFace++)
    {
      int i0 = iabs (m_pF[iFace].m_edge[0].v);
      int i1 = iabs (m_pF[iFace].m_edge[1].v);
      int i2 = iabs (m_pF[iFace].m_edge[2].v);
      int i3 = iabs (m_pF[iFace].m_edge[3].v);
      GeoTrf::Vector3D g;
      if (i3 == 0)
  {
    i3 = i0;
    g = (m_pV[i0] + m_pV[i1] + m_pV[i2]) * (1.0f / 3.0f);
  }
      else
  {
    g = (m_pV[i0] + m_pV[i1] + m_pV[i2] + m_pV[i3]) * 0.25f;
  }
      v += ((m_pV[i2] - m_pV[i0]).cross (m_pV[i3] - m_pV[i1])).dot (g);
    }
  return v * (1./6);
}

GeoPolyhedronTrd2::GeoPolyhedronTrd2 (double Dx1, double Dx2,
              double Dy1, double Dy2, double Dz)
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedronTrd2                           Date:    22.07.96 *
 * Author: E.Chernyaev (IHEP/Protvino)               Revised:          *
 *                                                                     *
 * Function: Create GEANT4 TRD2-trapezoid                              *
 *                                                                     *
 * Input: Dx1 - half-length along X at -Dz           8----7            *
 *        Dx2 - half-length along X ay +Dz        5----6  !            *
 *        Dy1 - half-length along Y ay -Dz        !  4-!--3            *
 *        Dy2 - half-length along Y ay +Dz        1----2               *
 *        Dz  - half-length along Z                                    *
 *                                                                     *
 ***********************************************************************/
{
  AllocateMemory (8, 6);

  m_pV[1] = GeoTrf::Vector3D (-Dx1, -Dy1, -Dz);
  m_pV[2] = GeoTrf::Vector3D (Dx1, -Dy1, -Dz);
  m_pV[3] = GeoTrf::Vector3D (Dx1, Dy1, -Dz);
  m_pV[4] = GeoTrf::Vector3D (-Dx1, Dy1, -Dz);
  m_pV[5] = GeoTrf::Vector3D (-Dx2, -Dy2, Dz);
  m_pV[6] = GeoTrf::Vector3D (Dx2, -Dy2, Dz);
  m_pV[7] = GeoTrf::Vector3D (Dx2, Dy2, Dz);
  m_pV[8] = GeoTrf::Vector3D (-Dx2, Dy2, Dz);

  CreatePrism ();
}

GeoPolyhedronTrd2::~GeoPolyhedronTrd2 ()
{
}

GeoPolyhedronTrd1::GeoPolyhedronTrd1 (double Dx1, double Dx2,
              double Dy, double Dz):
GeoPolyhedronTrd2 (Dx1, Dx2, Dy, Dy, Dz)
{
}

GeoPolyhedronTrd1::~GeoPolyhedronTrd1 ()
{
}

GeoPolyhedronBox::GeoPolyhedronBox (double Dx, double Dy, double Dz):
GeoPolyhedronTrd2 (Dx, Dx, Dy, Dy, Dz)
{
}

GeoPolyhedronBox::~GeoPolyhedronBox ()
{
}

GeoPolyhedronTrap::GeoPolyhedronTrap (double Dz,
              double Theta,
              double Phi,
              double Dy1,
              double Dx1,
              double Dx2,
              double Alp1,
              double Dy2,
              double Dx3, double Dx4, double Alp2)
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedronTrap                           Date:    20.11.96 *
 * Author: E.Chernyaev                               Revised:          *
 *                                                                     *
 * Function: Create GEANT4 TRAP-trapezoid                              *
 *                                                                     *
 * Input: DZ   - half-length in Z                                      *
 *        Theta,Phi - polar angles of the line joining centres of the  *
 *                    faces at Z=-Dz and Z=+Dz                         *
 *        Dy1  - half-length in Y of the face at Z=-Dz                 *
 *        Dx1  - half-length in X of low edge of the face at Z=-Dz     *
 *        Dx2  - half-length in X of top edge of the face at Z=-Dz     *
 *        Alp1 - angle between Y-axis and the median joining top and   *
 *               low edges of the face at Z=-Dz                        *
 *        Dy2  - half-length in Y of the face at Z=+Dz                 *
 *        Dx3  - half-length in X of low edge of the face at Z=+Dz     *
 *        Dx4  - half-length in X of top edge of the face at Z=+Dz     *
 *        Alp2 - angle between Y-axis and the median joining top and   *
 *               low edges of the face at Z=+Dz                        *
 *                                                                     *
 ***********************************************************************/
{
  double DzTthetaCphi = Dz * tan (Theta) * cos (Phi);
  double DzTthetaSphi = Dz * tan (Theta) * sin (Phi);
  double Dy1Talp1 = Dy1 * tan (Alp1);
  double Dy2Talp2 = Dy2 * tan (Alp2);

  AllocateMemory (8, 6);

  m_pV[1] =
    GeoTrf::Vector3D (-DzTthetaCphi - Dy1Talp1 - Dx1, -DzTthetaSphi - Dy1, -Dz);
  m_pV[2] =
    GeoTrf::Vector3D (-DzTthetaCphi - Dy1Talp1 + Dx1, -DzTthetaSphi - Dy1, -Dz);
  m_pV[3] =
    GeoTrf::Vector3D (-DzTthetaCphi + Dy1Talp1 + Dx2, -DzTthetaSphi + Dy1, -Dz);
  m_pV[4] =
    GeoTrf::Vector3D (-DzTthetaCphi + Dy1Talp1 - Dx2, -DzTthetaSphi + Dy1, -Dz);
  m_pV[5] = GeoTrf::Vector3D (DzTthetaCphi - Dy2Talp2 - Dx3, DzTthetaSphi - Dy2, Dz);
  m_pV[6] = GeoTrf::Vector3D (DzTthetaCphi - Dy2Talp2 + Dx3, DzTthetaSphi - Dy2, Dz);
  m_pV[7] = GeoTrf::Vector3D (DzTthetaCphi + Dy2Talp2 + Dx4, DzTthetaSphi + Dy2, Dz);
  m_pV[8] = GeoTrf::Vector3D (DzTthetaCphi + Dy2Talp2 - Dx4, DzTthetaSphi + Dy2, Dz);

  CreatePrism ();
}

GeoPolyhedronTrap::~GeoPolyhedronTrap ()
{
}

GeoPolyhedronTwistedTrap::GeoPolyhedronTwistedTrap (double TwistPhi, double Dz,
              double Theta,
              double Phi,
              double Dy1,
              double Dx1,
              double Dx2,
              double Dy2,
              double Dx3,
              double Dx4,
              double Alp)
/***********************************************************************
 *
 * Name: GeoPolyhedronTwistedTrap         Date:    26.11.2020
 * Author: Marilena Bandieramonte
 *
 * Function: Create TwistedTrap - trapezoid for visualization
 *
 * A TwistedTrap is a general twisted trapezoid: The faces perpendicular to the
 * z planes are trapezia, and their centres are not necessarily on
 * a line parallel to the z axis.
 *
 *      TwistPhi  Phi twist angle
 *      Dz      Half-length along the z-axis
 *      Theta  Polar angle of the line joining the centres of the faces at -/+pDz
 *      Phi     Azimuthal angle of the line joing the centre of the face at -pDz to the centre of the face at +pDz
 *      Dy1    Half-length along y of the face at -pDz
 *      Dx1    Half-length along x of the side at y=-pDy1 of the face at -pDz
 *      Dx2    Half-length along x of the side at y=+pDy1 of the face at -pDz
 *
 *      Dy2    Half-length along y of the face at +pDz
 *      Dx3    Half-length along x of the side at y=-pDy2 of the face at +pDz
 *      Dx4    Half-length along x of the side at y=+pDy2 of the face at +pDz
 *      Alp   Angle with respect to the y axis from the centre of the side
 *
 ***********************************************************************/
{
    AllocateMemory(12,18);
    std::cout<<"Creating a GeoPolyhedronTwistedTrap"<<std::endl;
    float xy1[4][2]; //quadrilateral at the bottom -DZ
                     //     2 ------ 3
                     //        1 ------ 4
    float xy2[4][2]; //quadrilateral at the top    +DZ
                     //     6 ------ 7
                     //        5 ------ 8
    
    const float tanTheta(tan(Theta));
    const float TthetaCphi = tanTheta*cos(Phi);
    const float TthetaSphi = tanTheta*sin(Phi);
    const float Talp = tan(Alp);
    
    //BOTTOM - clockwise
    xy1[0][0]= -Dx2+Dy1*Talp; //5
    xy1[0][1]=  Dy1;
    xy1[1][0]= -Dx1-Dy1*Talp; //6
    xy1[1][1]= -Dy1;
    xy1[2][0]=  Dx1-Dy1*Talp; //7
    xy1[2][1]= -Dy1;
    xy1[3][0]=  Dx2+Dy1*Talp; //8
    xy1[3][1]=  Dy1;

    //TOP - clockwise
    xy2[0][0]= -Dx4+Dy2*Talp; //1
    xy2[0][1]=  Dy2;
    xy2[1][0]= -Dx3-Dy2*Talp; //2
    xy2[1][1]= -Dy2;
    xy2[2][0]=  Dx3-Dy2*Talp; //3
    xy2[2][1]= -Dy2;
    xy2[3][0]=  Dx4+Dy2*Talp; //4
    xy2[3][1]=  Dy2;

    const float dzTthetaCphi(Dz*TthetaCphi);
    const float dzTthetaSphi(Dz*TthetaSphi);
    
    for (int i=0;i<4;i++) {
      xy1[i][0]-=dzTthetaCphi;
      xy1[i][1]-=dzTthetaSphi;
      xy2[i][0]+=dzTthetaCphi;
      xy2[i][1]+=dzTthetaSphi;
    }

    float xtmp, ytmp;
    const float cPhiTwist=cos(TwistPhi);
    const float sPhiTwist=sin(TwistPhi);
    
    //Apply twist (rotate aroud Z of an angle TwistPhi) to the top surface only
    for (int i=0;i<4;i++) {
      xtmp =xy2[i][0];
      ytmp =xy2[i][1];
      xy2[i][0]= xtmp * cPhiTwist - ytmp * sPhiTwist;
      xy2[i][1]= xtmp * sPhiTwist + ytmp * cPhiTwist;
    }
    
    m_pV[ 1] = GeoTrf::Vector3D(xy1[0][0],xy1[0][1],-Dz);
    m_pV[ 2] = GeoTrf::Vector3D(xy1[1][0],xy1[1][1],-Dz);
    m_pV[ 3] = GeoTrf::Vector3D(xy1[2][0],xy1[2][1],-Dz);
    m_pV[ 4] = GeoTrf::Vector3D(xy1[3][0],xy1[3][1],-Dz);

    m_pV[ 5] = GeoTrf::Vector3D(xy2[0][0],xy2[0][1], Dz);
    m_pV[ 6] = GeoTrf::Vector3D(xy2[1][0],xy2[1][1], Dz);
    m_pV[ 7] = GeoTrf::Vector3D(xy2[2][0],xy2[2][1], Dz);
    m_pV[ 8] = GeoTrf::Vector3D(xy2[3][0],xy2[3][1], Dz);
    
     m_pV[ 9] = (m_pV[1]+m_pV[2]+m_pV[5]+m_pV[6])/4.;
     m_pV[10] = (m_pV[2]+m_pV[3]+m_pV[6]+m_pV[7])/4.;
     m_pV[11] = (m_pV[3]+m_pV[4]+m_pV[7]+m_pV[8])/4.;
     m_pV[12] = (m_pV[4]+m_pV[1]+m_pV[8]+m_pV[5])/4.;

     enum {DUMMY, BOTTOM,
           LEFT_BOTTOM,  LEFT_FRONT,   LEFT_TOP,  LEFT_BACK,
           BACK_BOTTOM,  BACK_LEFT,    BACK_TOP,  BACK_RIGHT,
           RIGHT_BOTTOM, RIGHT_BACK,   RIGHT_TOP, RIGHT_FRONT,
           FRONT_BOTTOM, FRONT_RIGHT,  FRONT_TOP, FRONT_LEFT,
           TOP};

     m_pF[ 1]=GeoFacet(1,LEFT_BOTTOM, 4,BACK_BOTTOM, 3,RIGHT_BOTTOM, 2,FRONT_BOTTOM);

     m_pF[ 2]=GeoFacet(4,BOTTOM,     -1,LEFT_FRONT,  -12,LEFT_BACK,    0,0);
     m_pF[ 3]=GeoFacet(1,FRONT_LEFT, -5,LEFT_TOP,    -12,LEFT_BOTTOM,  0,0);
     m_pF[ 4]=GeoFacet(5,TOP,        -8,LEFT_BACK,   -12,LEFT_FRONT,   0,0);
     m_pF[ 5]=GeoFacet(8,BACK_LEFT,  -4,LEFT_BOTTOM, -12,LEFT_TOP,     0,0);

     m_pF[ 6]=GeoFacet(3,BOTTOM,     -4,BACK_LEFT,   -11,BACK_RIGHT,   0,0);
     m_pF[ 7]=GeoFacet(4,LEFT_BACK,  -8,BACK_TOP,    -11,BACK_BOTTOM,  0,0);
     m_pF[ 8]=GeoFacet(8,TOP,        -7,BACK_RIGHT,  -11,BACK_LEFT,    0,0);
     m_pF[ 9]=GeoFacet(7,RIGHT_BACK, -3,BACK_BOTTOM, -11,BACK_TOP,     0,0);

     m_pF[10]=GeoFacet(2,BOTTOM,     -3,RIGHT_BACK,  -10,RIGHT_FRONT,  0,0);
     m_pF[11]=GeoFacet(3,BACK_RIGHT, -7,RIGHT_TOP,   -10,RIGHT_BOTTOM, 0,0);
     m_pF[12]=GeoFacet(7,TOP,        -6,RIGHT_FRONT, -10,RIGHT_BACK,   0,0);
     m_pF[13]=GeoFacet(6,FRONT_RIGHT,-2,RIGHT_BOTTOM,-10,RIGHT_TOP,    0,0);

     m_pF[14]=GeoFacet(1,BOTTOM,     -2,FRONT_RIGHT,  -9,FRONT_LEFT,   0,0);
     m_pF[15]=GeoFacet(2,RIGHT_FRONT,-6,FRONT_TOP,    -9,FRONT_BOTTOM, 0,0);
     m_pF[16]=GeoFacet(6,TOP,        -5,FRONT_LEFT,   -9,FRONT_RIGHT,  0,0);
     m_pF[17]=GeoFacet(5,LEFT_FRONT, -1,FRONT_BOTTOM, -9,FRONT_TOP,    0,0);

     m_pF[18]=GeoFacet(5,FRONT_TOP, 6,RIGHT_TOP, 7,BACK_TOP, 8,LEFT_TOP);
    
}

GeoPolyhedronTwistedTrap::~GeoPolyhedronTwistedTrap ()
{
}

GeoPolyhedronPara::GeoPolyhedronPara (double Dx, double Dy, double Dz,
              double Alpha, double Theta, double Phi):
GeoPolyhedronTrap (Dz, Theta, Phi, Dy, Dx, Dx, Alpha, Dy, Dx, Dx, Alpha)
{
}

GeoPolyhedronPara::~GeoPolyhedronPara ()
{
}

GeoPolyhedronCons::GeoPolyhedronCons (double Rmn1,
              double Rmx1,
              double Rmn2,
              double Rmx2,
              double Dz, double Phi1, double Dphi)
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedronCons::GeoPolyhedronCons        Date:    15.12.96 *
 * Author: E.Chernyaev (IHEP/Protvino)               Revised: 15.12.96 *
 *                                                                     *
 * Function: Constructor for CONS, TUBS, CONE, TUBE                    *
 *                                                                     *
 * Input: Rmn1, Rmx1 - inside and outside radiuses at -Dz              *
 *        Rmn2, Rmx2 - inside and outside radiuses at +Dz              *
 *        Dz         - half length in Z                                *
 *        Phi1       - starting angle of the segment                   *
 *        Dphi       - segment range                                   *
 *                                                                     *
 ***********************************************************************/
{
  static double wholeCircle = 2 * M_PI;

  //   C H E C K   I N P U T   P A R A M E T E R S

  int k = 0;
  if (Rmn1 < 0. || Rmx1 < 0. || Rmn2 < 0. || Rmx2 < 0.)
    k = 1;
  if (Rmn1 > Rmx1 || Rmn2 > Rmx2)
    k = 1;
  if (Rmn1 == Rmx1 && Rmn2 == Rmx2)
    k = 1;

  if (Dz <= 0.)
    k += 2;

  double phi1, phi2, dphi;
  if (Dphi < 0.)
    {
      phi2 = Phi1;
      phi1 = phi2 - Dphi;
    }
  else if (Dphi == 0.)
    {
      phi1 = Phi1;
      phi2 = phi1 + wholeCircle;
    }
  else
    {
      phi1 = Phi1;
      phi2 = phi1 + Dphi;
    }
  dphi = phi2 - phi1;
  if (fabs (dphi - wholeCircle) < perMillion)
    dphi = wholeCircle;
  if (dphi > wholeCircle)
    k += 4;

  if (k != 0)
    {
      std::cerr << "GeoPolyhedronCone(s)/Tube(s): error in input parameters";
      if ((k & 1) != 0)
  std::cerr << " (radiuses)";
      if ((k & 2) != 0)
  std::cerr << " (half-length)";
      if ((k & 4) != 0)
  std::cerr << " (angles)";
      std::cerr << std::endl;
      std::cerr << " Rmn1=" << Rmn1 << " Rmx1=" << Rmx1;
      std::cerr << " Rmn2=" << Rmn2 << " Rmx2=" << Rmx2;
      std::cerr << " Dz=" << Dz << " Phi1=" << Phi1 << " Dphi=" << Dphi
  << std::endl;
      return;
    }

  //   P R E P A R E   T W O   P O L Y L I N E S

  double zz[4], rr[4];
  zz[0] = Dz;
  zz[1] = -Dz;
  zz[2] = Dz;
  zz[3] = -Dz;
  rr[0] = Rmx2;
  rr[1] = Rmx1;
  rr[2] = Rmn2;
  rr[3] = Rmn1;

  //   R O T A T E    P O L Y L I N E S

  RotateAroundZ (0, phi1, dphi, 2, 2, zz, rr, -1, -1);
  SetReferences ();
}

GeoPolyhedronCons::~GeoPolyhedronCons ()
{
}

GeoPolyhedronCone::GeoPolyhedronCone (double Rmn1, double Rmx1,
              double Rmn2, double Rmx2, double Dz):
GeoPolyhedronCons (Rmn1, Rmx1, Rmn2, Rmx2, Dz, 0 * deg, 360 * deg)
{
}

GeoPolyhedronCone::~GeoPolyhedronCone ()
{
}

GeoPolyhedronTubs::GeoPolyhedronTubs (double Rmin, double Rmax,
              double Dz, double Phi1, double Dphi):
GeoPolyhedronCons (Rmin, Rmax, Rmin, Rmax, Dz, Phi1, Dphi)
{
}

GeoPolyhedronTubs::~GeoPolyhedronTubs ()
{
}

GeoPolyhedronTube::GeoPolyhedronTube (double Rmin, double Rmax, double Dz):
GeoPolyhedronCons (Rmin, Rmax, Rmin, Rmax, Dz, 0 * deg, 360 * deg)
{
}

GeoPolyhedronTube::~GeoPolyhedronTube ()
{
}

GeoPolyhedronPgon::GeoPolyhedronPgon (double phi,
              double dphi,
              int npdv,
              int nz,
              const double *z,
              const double *rmin, const double *rmax)
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedronPgon                           Date:    09.12.96 *
 * Author: E.Chernyaev                               Revised:          *
 *                                                                     *
 * Function: Constructor of polyhedron for PGON, PCON                  *
 *                                                                     *
 * Input: phi  - initial phi                                           *
 *        dphi - delta phi                                             *
 *        npdv - number of steps along phi                             *
 *        nz   - number of z-planes (at least two)                     *
 *        z[]  - z coordinates of the slices                           *
 *        rmin[] - smaller r at the slices                             *
 *        rmax[] - bigger  r at the slices                             *
 *                                                                     *
 ***********************************************************************/
{
  //   C H E C K   I N P U T   P A R A M E T E R S

  if (dphi <= 0. || dphi > 2 * M_PI)
    {
      std::cerr
  << "GeoPolyhedronPgon/Pcon: wrong delta phi = " << dphi << std::endl;
      return;
    }

  if (nz < 2)
    {
      std::cerr
  << "GeoPolyhedronPgon/Pcon: number of z-planes less than two = " << nz
  << std::endl;
      return;
    }

  if (npdv < 0)
    {
      std::cerr
  << "GeoPolyhedronPgon/Pcon: error in number of phi-steps =" << npdv
  << std::endl;
      return;
    }

  int i;
  for (i = 0; i < nz; i++)
    {
      if (rmin[i] < 0. || rmax[i] < 0. || rmin[i] > rmax[i])
  {
    std::cerr
      << "GeoPolyhedronPgon: error in radiuses rmin[" << i << "]="
      << rmin[i] << " rmax[" << i << "]=" << rmax[i] << std::endl;
    return;
  }
    }

  //   P R E P A R E   T W O   P O L Y L I N E S

  double *zz, *rr;
  zz = new double[2 * nz];
  rr = new double[2 * nz];

  if (z[0] > z[nz - 1])
    {
      for (i = 0; i < nz; i++)
  {
    zz[i] = z[i];
    rr[i] = rmax[i];
    zz[i + nz] = z[i];
    rr[i + nz] = rmin[i];
  }
    }
  else
    {
      for (i = 0; i < nz; i++)
  {
    zz[i] = z[nz - i - 1];
    rr[i] = rmax[nz - i - 1];
    zz[i + nz] = z[nz - i - 1];
    rr[i + nz] = rmin[nz - i - 1];
  }
    }

  //   R O T A T E    P O L Y L I N E S

  RotateAroundZ (npdv, phi, dphi, nz, nz, zz, rr, -1, (npdv == 0) ? -1 : 1);
  SetReferences ();

  delete[]zz;
  delete[]rr;
}

GeoPolyhedronPgon::~GeoPolyhedronPgon ()
{
}

GeoPolyhedronPcon::GeoPolyhedronPcon (double phi, double dphi, int nz,
              const double *z,
              const double *rmin, const double *rmax):
GeoPolyhedronPgon (phi, dphi, 0, nz, z, rmin, rmax)
{
}

GeoPolyhedronPcon::~GeoPolyhedronPcon ()
{
}

GeoPolyhedronSphere::GeoPolyhedronSphere (double rmin, double rmax,
            double phi, double dphi,
            double the, double dthe)
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedronSphere                         Date:    11.12.96 *
 * Author: E.Chernyaev (IHEP/Protvino)               Revised:          *
 *                                                                     *
 * Function: Constructor of polyhedron for SPHERE                      *
 *                                                                     *
 * Input: rmin - internal radius                                       *
 *        rmax - external radius                                       *
 *        phi  - initial phi                                           *
 *        dphi - delta phi                                             *
 *        the  - initial theta                                         *
 *        dthe - delta theta                                           *
 *                                                                     *
 ***********************************************************************/
{
  //   C H E C K   I N P U T   P A R A M E T E R S

  if (dphi <= 0. || dphi > 2 * M_PI)
    {
      std::cerr
  << "GeoPolyhedronSphere: wrong delta phi = " << dphi << std::endl;
      return;
    }

  if (the < 0. || the > M_PI)
    {
      std::cerr << "GeoPolyhedronSphere: wrong theta = " << the << std::endl;
      return;
    }

  if (dthe <= 0. || dthe > M_PI)
    {
      std::cerr
  << "GeoPolyhedronSphere: wrong delta theta = " << dthe << std::endl;
      return;
    }

  if ((the + dthe >= M_PI) && (the + dthe < M_PI + 2 * DBL_EPSILON))
    dthe = M_PI - the;    //G.Barrand : coming from LHCb/S.Ponce.

  if (the + dthe > M_PI)
    {
      std::cerr
  << "GeoPolyhedronSphere: wrong theta + delta theta = "
  << the << " " << dthe << std::endl;
      return;
    }

  if (rmin < 0. || rmin >= rmax)
    {
      std::cerr
  << "GeoPolyhedronSphere: error in radiuses"
  << " rmin=" << rmin << " rmax=" << rmax << std::endl;
      return;
    }

  //   P R E P A R E   T W O   P O L Y L I N E S

  int ns = (GetNumberOfRotationSteps () + 1) / 2;
  int np1 = int (dthe * ns * M_1_PI + .5) + 1;
  if (np1 <= 1)
    np1 = 2;
  int np2 = rmin < perMillion ? 1 : np1;

  double *zz, *rr;
  zz = new double[np1 + np2];
  rr = new double[np1 + np2];

  double a = dthe / (np1 - 1);
  double cosa, sina;
  for (int i = 0; i < np1; i++)
    {
      cosa = cos (the + i * a);
      sina = sin (the + i * a);
      zz[i] = rmax * cosa;
      rr[i] = rmax * sina;
      if (np2 > 1)
  {
    zz[i + np1] = rmin * cosa;
    rr[i + np1] = rmin * sina;
  }
    }
  if (np2 == 1)
    {
      zz[np1] = 0.;
      rr[np1] = 0.;
    }

  //   R O T A T E    P O L Y L I N E S

  RotateAroundZ (0, phi, dphi, np1, np2, zz, rr, -1, -1);
  SetReferences ();

  delete[]zz;
  delete[]rr;
}

GeoPolyhedronSphere::~GeoPolyhedronSphere ()
{
}

GeoPolyhedronTorus::GeoPolyhedronTorus (double rmin,
          double rmax,
          double rtor, double phi, double dphi)
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedronTorus                          Date:    11.12.96 *
 * Author: E.Chernyaev (IHEP/Protvino)               Revised:          *
 *                                                                     *
 * Function: Constructor of polyhedron for TORUS                       *
 *                                                                     *
 * Input: rmin - internal radius                                       *
 *        rmax - external radius                                       *
 *        rtor - radius of torus                                       *
 *        phi  - initial phi                                           *
 *        dphi - delta phi                                             *
 *                                                                     *
 ***********************************************************************/
{
  //   C H E C K   I N P U T   P A R A M E T E R S

  if (dphi <= 0. || dphi > 2 * M_PI)
    {
      std::cerr
  << "GeoPolyhedronTorus: wrong delta phi = " << dphi << std::endl;
      return;
    }

  if (rmin < 0. || rmin >= rmax || rmax >= rtor)
    {
      std::cerr
  << "GeoPolyhedronTorus: error in radiuses"
  << " rmin=" << rmin << " rmax=" << rmax << " rtorus=" << rtor
  << std::endl;
      return;
    }

  //   P R E P A R E   T W O   P O L Y L I N E S

  int np1 = GetNumberOfRotationSteps ();
  int np2 = rmin < perMillion ? 1 : np1;

  std::vector<double> rr (np1+np2, 0);
  std::vector<double> zz (np1+np2, 0);

  double a = 2 * M_PI / np1;
  double cosa, sina;
  for (int i = 0; i < np1; i++)
    {
      cosa = cos (i * a);
      sina = sin (i * a);
      zz[i] = rmax * cosa;
      rr[i] = rtor + rmax * sina;
      if (np2 > 1)
  {
    zz[i + np1] = rmin * cosa;
    rr[i + np1] = rtor + rmin * sina;
  }
    }
  if (np2 == 1)
    {
      zz[np1] = 0.;
      rr[np1] = rtor;
      np2 = -1;
    }

  //   R O T A T E    P O L Y L I N E S

  RotateAroundZ (0, phi, dphi, -np1, -np2, zz.data(), rr.data(), -1, -1);
  SetReferences ();
}

GeoPolyhedronTorus::~GeoPolyhedronTorus ()
{
}

int
  GeoPolyhedron::s_fNumberOfRotationSteps =
  DEFAULT_NUMBER_OF_STEPS;
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedron::s_fNumberOfRotationSteps     Date:    24.06.97 *
 * Author: J.Allison (Manchester University)         Revised:          *
 *                                                                     *
 * Function: Number of steps for whole circle                          *
 *                                                                     *
 ***********************************************************************/

#include "BooleanProcessor.src"
static
  Geo_BooleanProcessor
  processor;

GeoPolyhedron GeoPolyhedron::add (const GeoPolyhedron & p) const
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedron::add                          Date:    19.03.00 *
 * Author: E.Chernyaev                               Revised:          *
 *                                                                     *
 * Function: Boolean "union" of two polyhedra                          *
 *                                                                     *
 ***********************************************************************/
{
  return processor.execute (OP_UNION, *this, p);
}

GeoPolyhedron GeoPolyhedron::intersect (const GeoPolyhedron & p) const
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedron::intersect                    Date:    19.03.00 *
 * Author: E.Chernyaev                               Revised:          *
 *                                                                     *
 * Function: Boolean "intersection" of two polyhedra                   *
 *                                                                     *
 ***********************************************************************/
{
  return processor.execute (OP_INTERSECTION, *this, p);
}

GeoPolyhedron GeoPolyhedron::subtract (const GeoPolyhedron & p) const
/***********************************************************************
 *                                                                     *
 * Name: GeoPolyhedron::add                          Date:    19.03.00 *
 * Author: E.Chernyaev                               Revised:          *
 *                                                                     *
 * Function: Boolean "subtraction" of "p" from "this"                  *
 *                                                                     *
 ***********************************************************************/
{
  return processor.execute (OP_SUBTRACTION, *this, p);
}
