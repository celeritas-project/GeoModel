#include "GeoModelKernel/GeoUnidentifiedShape.h"
#include "GeoModelKernel/Query.h"
#include "GeoModelKernel/GeoShapeAction.h"

const std::string         GeoUnidentifiedShape::_classType="UnidentifiedShape";
const ShapeType           GeoUnidentifiedShape::_classTypeID=0xFFFFFFFF;

// Destructor:
GeoUnidentifiedShape::~GeoUnidentifiedShape() {
}

// Constructor:
GeoUnidentifiedShape::GeoUnidentifiedShape(const std::string & name):
  _name(name) {}

// Constructor:
GeoUnidentifiedShape::GeoUnidentifiedShape(const std::string & name, const std::string & asciiData) :
  _name(name),
  _asciiData(asciiData) {}

// Constructor with volume: 
GeoUnidentifiedShape::GeoUnidentifiedShape(const std::string & name, const std::string & asciiData, double volume):
  _name(name),
  _asciiData(asciiData),
  _volume(volume) {}

// Returns the user-provided name of the volume (eg "MySpecialShape");
const std::string & GeoUnidentifiedShape::name() const {
  return _name;
}

// Returns the ascii data associated with this object (possibly empty);
const std::string & GeoUnidentifiedShape::asciiData() const {
  return _asciiData;
}

// Returns the volume of the shape, for mass inventory
double GeoUnidentifiedShape::volume () const {
  return _volume;
}
  
// Returns the shape type, as a string.
const std::string & GeoUnidentifiedShape::type () const {
  return _classType;
}

// Returns the shape type, as an coded integer.
ShapeType GeoUnidentifiedShape::typeID () const {
  return _classTypeID;
}

// Executes a GeoShapeAction
void GeoUnidentifiedShape::exec (GeoShapeAction *action) const {
  action->handleUnidentifiedShape(this);
}
