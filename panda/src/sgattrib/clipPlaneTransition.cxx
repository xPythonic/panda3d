// Filename: clipPlaneTransition.cxx
// Created by:  mike (06Feb99)
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

#include "clipPlaneTransition.h"
#include "clipPlaneAttribute.h"

#include <indent.h>

TypeHandle ClipPlaneTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated ClipPlaneTransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *ClipPlaneTransition::
make_copy() const {
  return new ClipPlaneTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneTransition::make_attrib
//       Access: Public, Virtual
//  Description: Returns a newly allocated ClipPlaneAttribute.
////////////////////////////////////////////////////////////////////
NodeAttribute *ClipPlaneTransition::
make_attrib() const {
  return new ClipPlaneAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneTransition::make_identity
//       Access: Public, Virtual
//  Description: Returns a newly allocated ClipPlaneTransition in the
//               initial state.
////////////////////////////////////////////////////////////////////
NodeTransition *ClipPlaneTransition::
make_identity() const {
  return new ClipPlaneTransition;
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneTransition::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void ClipPlaneTransition::
output_property(ostream &out, const PT_Node &prop) const {
  const PlaneNode *node;
  DCAST_INTO_V(node, prop.p());
  out << node->get_name() << "=" << node->get_plane();
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneTransition::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void ClipPlaneTransition::
write_property(ostream &out, const PT_Node &prop,
               int indent_level) const {
  const PlaneNode *node;
  DCAST_INTO_V(node, prop);
  indent(out, indent_level)
    << node->get_name() << "=" << node->get_plane() << "\n";
}
