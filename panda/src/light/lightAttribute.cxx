// Filename: lightAttribute.cxx
// Created by:  drose (24Mar00)
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

#include "lightAttribute.h"
#include "lightTransition.h"

#include <graphicsStateGuardianBase.h>
#include <indent.h>

TypeHandle LightAttribute::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LightAttribute::get_handle
//       Access: Public, Virtual
//  Description: Returns the handle of the associated transition.
////////////////////////////////////////////////////////////////////
TypeHandle LightAttribute::
get_handle() const {
  return LightTransition::get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttribute::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated LightAttribute just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeAttribute *LightAttribute::
make_copy() const {
  return new LightAttribute(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttribute::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated LightAttribute
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeAttribute *LightAttribute::
make_initial() const {
  return new LightAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttribute::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void LightAttribute::
issue(GraphicsStateGuardianBase *gsgbase) {
  gsgbase->issue_light(this);
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttribute::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void LightAttribute::
output_property(ostream &out, const PT_Light &prop) const {
  out << *prop;
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttribute::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void LightAttribute::
write_property(ostream &out, const PT_Light &prop,
               int indent_level) const {
  prop->write(out, indent_level);
}
