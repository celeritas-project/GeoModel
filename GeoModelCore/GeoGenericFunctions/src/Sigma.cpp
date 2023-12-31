//==========================================================================//
// Imported from the QAT library (qat.pitt.edu) by the copyright holder     //
// Joe Boudreau.  Software licensed under the terms and conditions of the   //
// GNU Lesser Public License v3.                                            //
//==========================================================================//
#include "GeoGenericFunctions/Sigma.h"
#include <stdexcept>

namespace GeoGenfun {
FUNCTION_OBJECT_IMP(Sigma)

void Sigma::accumulate( const AbsFunction & fcn) {
  _fcn.push_back(fcn.clone());
}

Sigma::Sigma()
{
}

Sigma::Sigma(const Sigma & right) : AbsFunction(right)
{
  for (size_t i=0; i<right._fcn.size();i++) {
    _fcn.push_back(right._fcn[i]->clone());
  }
}

unsigned int Sigma::dimensionality() const {
  if (_fcn.size()==0) throw std::runtime_error("Cannot determine dimensionality from empty sum");
  return _fcn[0]->dimensionality();
}

Sigma::~Sigma()
{
  for (size_t i=0; i<_fcn.size();i++) {
    delete _fcn[i];
  }
}

double Sigma::operator ()(double x) const
{
  double retVal=0.0;
  for (size_t i=0;i<_fcn.size();i++) retVal += (*_fcn[i])(x);
  return retVal;
}


double Sigma::operator ()(const Argument & x) const
{
  double retVal=0.0;
  for (size_t i=0;i<_fcn.size();i++) retVal += (*_fcn[i])(x);
  return retVal;
}



Derivative Sigma::partial(unsigned int index) const {
  Sigma fPrime;
  for (size_t i=0;i<_fcn.size();i++) {
    fPrime.accumulate(_fcn[i]->partial(index));
  }
  return Derivative(&fPrime);
}



} // namespace GeoGenfun
