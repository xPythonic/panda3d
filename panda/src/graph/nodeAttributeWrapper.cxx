// Filename: nodeAttributeWrapper.cxx
// Created by:  drose (20Mar00)
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

#include "nodeAttributeWrapper.h"
#include "nodeTransitionWrapper.h"

#include <indent.h>

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributeWrapper::init_from
//       Access: Public, Static
//  Description: This is a named constructor that creates an empty
//               NodeAttributeWrapper ready to access the same type
//               of NodeAttribute as the other.
////////////////////////////////////////////////////////////////////
NodeAttributeWrapper NodeAttributeWrapper::
init_from(const NodeTransitionWrapper &trans) {
  return NodeAttributeWrapper(trans.get_handle());
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributeWrapper::apply_in_place
//       Access: Public
//  Description: Modifies the attribute by applying the transition.
////////////////////////////////////////////////////////////////////
void NodeAttributeWrapper::
apply_in_place(const NodeTransitionWrapper &trans) {
  nassertv(_handle == trans.get_handle());
  _attrib = NodeTransitionCacheEntry::apply(_attrib, trans._entry);
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributeWrapper::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void NodeAttributeWrapper::
output(ostream &out) const {
  if (_attrib == (NodeAttribute *)NULL) {
    out << "no " << _handle;
  } else {
    out << *_attrib;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodeAttributeWrapper::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void NodeAttributeWrapper::
write(ostream &out, int indent_level) const {
  if (_attrib == (NodeAttribute *)NULL) {
    indent(out, indent_level) << "no " << _handle << "\n";
  } else {
    _attrib->write(out, indent_level);
  }
}
