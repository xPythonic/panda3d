// Filename: plane_src.cxx
// Created by:  drose (03Apr01)
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


////////////////////////////////////////////////////////////////////
//     Function: Plane::get_reflection_mat
//       Access: Public
//  Description: This computes a transform matrix that performs the
//               perspective transform defined by the frustum,
//               accordinate to the indicated coordinate system.
////////////////////////////////////////////////////////////////////
FLOATNAME(LMatrix4) FLOATNAME(Plane)::
get_reflection_mat(void) const {
  FLOATTYPE aa = _a * _a; FLOATTYPE ab = _a * _b; FLOATTYPE ac = _a * _c;
  FLOATTYPE ad = _a * _d;
  FLOATTYPE bb = _b * _b; FLOATTYPE bc = _b * _c; FLOATTYPE bd = _b * _d;
  FLOATTYPE cc = _c * _c; FLOATTYPE cd = _c * _d;

  return FLOATNAME(LMatrix4)(  1-2*aa,  -2*ab,  -2*ac,     0,
                              -2*ab, 1-2*bb,  -2*bc,     0,
                              -2*ac,  -2*bc, 1-2*cc,     0,
                              -2*ad,  -2*bd,  -2*cd,     1  );
}

////////////////////////////////////////////////////////////////////
//     Function: Plane::get_point
//       Access: Public
//  Description: Returns an arbitrary point in the plane.  This can be
//               used along with the normal returned by get_normal()
//               to reconstruct the plane.
////////////////////////////////////////////////////////////////////
FLOATNAME(LPoint3) FLOATNAME(Plane)::
get_point() const {
  // Choose the denominator based on the largest axis in the normal.
  if (cabs(_a) >= cabs(_b) && cabs(_a) >= cabs(_c)) {
    nassertr(_a != 0.0f, FLOATNAME(LPoint3)(0.0f, 0.0f, 0.0f));
    return FLOATNAME(LPoint3)(-_d / _a, 0.0f, 0.0f);
  } else if (cabs(_b) >= cabs(_c)) {
    nassertr(_b != 0.0f, FLOATNAME(LPoint3)(0.0f, 0.0f, 0.0f));
    return FLOATNAME(LPoint3)(0.0f, -_d / _b, 0.0f);
  } else {
    nassertr(_c != 0.0f, FLOATNAME(LPoint3)(0.0f, 0.0f, 0.0f));
    return FLOATNAME(LPoint3)(0.0f, 0.0f, -_d / _c);
  }
}
