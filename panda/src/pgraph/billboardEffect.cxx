// Filename: billboardEffect.cxx
// Created by:  drose (14Mar02)
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

#include "billboardEffect.h"
#include "look_at.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle BillboardEffect::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BillboardEffect::make
//       Access: Published, Static
//  Description: Constructs a new BillboardEffect object with the
//               indicated properties.
////////////////////////////////////////////////////////////////////
CPT(RenderEffect) BillboardEffect::
make(const LVector3f &up_vector, bool eye_relative,
     bool axial_rotate, float offset, const qpNodePath &look_at,
     const LPoint3f &look_at_point) {
  BillboardEffect *effect = new BillboardEffect;
  effect->_up_vector = up_vector;
  effect->_eye_relative = eye_relative;
  effect->_axial_rotate = axial_rotate;
  effect->_offset = offset;
  effect->_look_at = look_at;
  effect->_look_at_point = look_at_point;
  effect->_off = false;
  return return_new(effect);
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardEffect::safe_to_transform
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to transform
//               this particular kind of RenderEffect by calling the
//               xform() method, false otherwise.
////////////////////////////////////////////////////////////////////
bool BillboardEffect::
safe_to_transform() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardEffect::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void BillboardEffect::
output(ostream &out) const {
  out << get_type() << ":";
  if (is_off()) {
    out << "(off)";
  } else {
    if (_axial_rotate) {
      out << "(axis";
    } else {
      out << "(point";
    }
    if (!_up_vector.almost_equal(LVector3f::up())) {
      out << " up " << _up_vector;
    }
    if (_eye_relative) {
      out << " eye";
    }
    if (_offset != 0.0f) {
      out << " offset " << _offset;
    }
    if (!_look_at.is_empty()) {
      out << " look at " << _look_at;
    }
    if (!_look_at_point.almost_equal(LPoint3f(0.0f, 0.0f, 0.0f))) {
      out << " look at point " << _look_at_point;
    }
    out << ")";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardEffect::do_billboard
//       Access: Public
//  Description: Computes the appropriate transform to apply to the
//               billboarded geometry, given its current net
//               transform, and the camera's inverse net transform.
////////////////////////////////////////////////////////////////////
CPT(TransformState) BillboardEffect::
do_billboard(const TransformState *net_transform,
             const TransformState *camera_transform) const {
  // Determine the relative transform to our camera (or other look_at
  // coordinate space).
  CPT(TransformState) rel_transform =
    net_transform->invert_compose(camera_transform);
  const LMatrix4f &rel_mat = rel_transform->get_mat();

  // Determine the look_at point in the camera space.
  LVector3f camera_pos, up;

  // If this is an eye-relative Billboard, then (a) the up vector is
  // relative to the camera, not to the world, and (b) the look
  // direction is towards the plane that contains the camera,
  // perpendicular to the forward direction, not directly to the
  // camera.

  if (_eye_relative) {
    up = _up_vector * rel_mat;
    camera_pos = LVector3f::forward() * rel_mat;

  } else {
//  camera_pos= -rel_mat.get_row3(3);

    camera_pos[0] = -rel_mat(3,0);
    camera_pos[1] = -rel_mat(3,1);
    camera_pos[2] = -rel_mat(3,2);

    up = _up_vector;
  }

  // Now determine the rotation matrix for the Billboard.
  LMatrix4f rotate;
  if (_axial_rotate) {
    heads_up(rotate, camera_pos, up);
  } else {
    look_at(rotate, camera_pos, up);
  }

  // Also slide the billboard geometry towards the camera according to
  // the offset factor.
  if (_offset != 0.0f) {
    LVector3f translate(rel_mat(3, 0), rel_mat(3, 1), rel_mat(3, 2));
    translate.normalize();
    translate *= _offset;
    rotate.set_row(3, translate);
  }

  return TransformState::make_mat(rotate);
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardEffect::compare_to_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived BillboardEffect
//               types to return a unique number indicating whether
//               this BillboardEffect is equivalent to the other one.
//
//               This should return 0 if the two BillboardEffect objects
//               are equivalent, a number less than zero if this one
//               should be sorted before the other one, and a number
//               greater than zero otherwise.
//
//               This will only be called with two BillboardEffect
//               objects whose get_type() functions return the same.
////////////////////////////////////////////////////////////////////
int BillboardEffect::
compare_to_impl(const RenderEffect *other) const {
  const BillboardEffect *ta;
  DCAST_INTO_R(ta, other, 0);

  if (_axial_rotate != ta->_axial_rotate) {
    return _axial_rotate - ta->_axial_rotate;
  }
  if (_eye_relative != ta->_eye_relative) {
    return _eye_relative - ta->_eye_relative;
  }
  if (_offset != ta->_offset) {
    return _offset < ta->_offset ? -1 : 1;
  }
  int compare = _up_vector.compare_to(ta->_up_vector);
  if (compare != 0) {
    return compare;
  }
  compare = _look_at.compare_to(ta->_look_at);
  if (compare != 0) {
    return compare;
  }
  compare = _look_at_point.compare_to(ta->_look_at_point);
  if (compare != 0) {
    return compare;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardEffect::make_default_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived BillboardEffect
//               types to specify what the default property for a
//               BillboardEffect of this type should be.
//
//               This should return a newly-allocated BillboardEffect of
//               the same type that corresponds to whatever the
//               standard default for this kind of BillboardEffect is.
////////////////////////////////////////////////////////////////////
RenderEffect *BillboardEffect::
make_default_impl() const {
  return new BillboardEffect;
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardEffect::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               BillboardEffect.
////////////////////////////////////////////////////////////////////
void BillboardEffect::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardEffect::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void BillboardEffect::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderEffect::write_datagram(manager, dg);

  dg.add_bool(_off);
  _up_vector.write_datagram(dg);
  dg.add_bool(_eye_relative);
  dg.add_bool(_axial_rotate);
  dg.add_float32(_offset);
  _look_at_point.write_datagram(dg);

  // *** We don't write out the _look_at qpNodePath right now.  Maybe
  // we should.
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardEffect::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type BillboardEffect is encountered
//               in the Bam file.  It should create the BillboardEffect
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *BillboardEffect::
make_from_bam(const FactoryParams &params) {
  BillboardEffect *effect = new BillboardEffect;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  effect->fillin(scan, manager);

  return effect;
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardEffect::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new BillboardEffect.
////////////////////////////////////////////////////////////////////
void BillboardEffect::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderEffect::fillin(scan, manager);

  _off = scan.get_bool();
  _up_vector.read_datagram(scan);
  _eye_relative = scan.get_bool();
  _axial_rotate = scan.get_bool();
  _offset = scan.get_float32();
  _look_at_point.read_datagram(scan);
}
