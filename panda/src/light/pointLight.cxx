// Filename: pointLight.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////
#include "pointLight.h"

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle PointLight::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PointLight::Constructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
PointLight::PointLight(const string& name) : NamedNode(name)
{
  set_color(Colorf(1, 1, 1, 1));
  set_specular(Colorf(1, 1, 1, 1));

  set_constant_attenuation(1);
  set_linear_attenuation(0);
  set_quadratic_attenuation(0);
}

////////////////////////////////////////////////////////////////////
//     Function: PointLight::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void PointLight::
output(ostream &out) const {
  NamedNode::output(out);
}

////////////////////////////////////////////////////////////////////
//     Function: PointLight::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void PointLight::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << ":\n";
  indent(out, indent_level + 2) << "color " << _color << "\n";
  indent(out, indent_level + 2) << "specular " << _specular << "\n";
  indent(out, indent_level + 2)
    << "attenuation " << _constant_attenuation << ", "
    << _linear_attenuation << ", " << _quadratic_attenuation << "\n";
}
