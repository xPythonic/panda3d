// Filename: curveFitter.cxx
// Created by:  drose (17Sep98)
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

#include "pandabase.h"
#include "pointerTo.h"

#include "curveFitter.h"
#include "config_parametrics.h"
#include "parametricCurve.h"
#include "classicNurbsCurve.h"
#include "hermiteCurve.h"
#include <algorithm>

TypeHandle CurveFitter::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CurveFitter::
CurveFitter() {
  _got_xyz = false;
  _got_hpr = false;
}

////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CurveFitter::
~CurveFitter() {
}

////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::reset
//       Access: Public
//  Description: Removes all the data points previously added to the
//               CurveFitter, and initializes it for a new curve.
////////////////////////////////////////////////////////////////////
void CurveFitter::
reset() {
  _data.erase(_data.begin(), _data.end());
}

////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::add_xyz
//       Access: Public
//  Description: Adds a single sample xyz.
////////////////////////////////////////////////////////////////////
void CurveFitter::
add_xyz(float t, const LVecBase3f &xyz) {
  DataPoint dp;
  dp._t = t;
  dp._xyz = xyz;
  _data.push_back(dp);
  _got_xyz = true;
}

////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::add_hpr
//       Access: Public
//  Description: Adds a single sample hpr.
////////////////////////////////////////////////////////////////////
void CurveFitter::
add_hpr(float t, const LVecBase3f &hpr) {
  DataPoint dp;
  dp._t = t;
  dp._hpr = hpr;
  _data.push_back(dp);
  _got_hpr = true;
}

////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::add_xyz_hpr
//       Access: Public
//  Description: Adds a single sample xyz & hpr simultaneously.
////////////////////////////////////////////////////////////////////
void CurveFitter::
add_xyz_hpr(float t, const LVecBase3f &xyz, const LVecBase3f &hpr) {
  DataPoint dp;
  dp._t = t;
  dp._xyz = xyz;
  dp._hpr = hpr;
  _data.push_back(dp);
  _got_xyz = true;
  _got_hpr = true;
}

////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::get_num_samples
//       Access: Public
//  Description: Returns the number of sample points that have been
//               added.
////////////////////////////////////////////////////////////////////
int CurveFitter::
get_num_samples() const {
  return _data.size();
}

////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::get_sample_t
//       Access: Public
//  Description: Returns the parametric value of the nth sample added.
////////////////////////////////////////////////////////////////////
float CurveFitter::
get_sample_t(int n) const {
  nassertr(n >= 0 && n < (int)_data.size(), 0.0);
  return _data[n]._t;
}

////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::get_sample_xyz
//       Access: Public
//  Description: Returns the point in space of the nth sample added.
////////////////////////////////////////////////////////////////////
LVecBase3f CurveFitter::
get_sample_xyz(int n) const {
  nassertr(n >= 0 && n < (int)_data.size(), LVecBase3f::zero());
  return _data[n]._xyz;
}

////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::get_sample_hpr
//       Access: Public
//  Description: Returns the orientation of the nth sample added.
////////////////////////////////////////////////////////////////////
LVecBase3f CurveFitter::
get_sample_hpr(int n) const {
  nassertr(n >= 0 && n < (int)_data.size(), LVecBase3f::zero());
  return _data[n]._hpr;
}

////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::get_sample_tangent
//       Access: Public
//  Description: Returns the tangent associated with the nth sample
//               added.  This is only meaningful if compute_tangents()
//               has already been called.
////////////////////////////////////////////////////////////////////
LVecBase3f CurveFitter::
get_sample_tangent(int n) const {
  nassertr(n >= 0 && n < (int)_data.size(), LVecBase3f::zero());
  return _data[n]._tangent;
}

////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::remove_samples
//       Access: Public
//  Description: Eliminates all samples from index begin, up to but not
//               including index end, from the database.
////////////////////////////////////////////////////////////////////
void CurveFitter::
remove_samples(int begin, int end) {
  begin = max(0, min((int)_data.size(), begin));
  end = max(0, min((int)_data.size(), end));

  nassertv(begin <= end);

  _data.erase(_data.begin() + begin, _data.begin() + end);
}

////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::sample
//       Access: Public
//  Description: Generates a series of data points by sampling the
//               given curve (or xyz/hpr curves) the indicated number
//               of times.  The sampling is made evenly in parametric
//               time, and then the timewarps, if any, are applied.
////////////////////////////////////////////////////////////////////
void CurveFitter::
sample(ParametricCurveCollection *curves, int count) {
  nassertv(curves != (ParametricCurveCollection *)NULL);
  float max_t = curves->get_max_t();
  float t, last_t, d;
  DataPoint dp;

  last_t = 0.0;
  d = 0.0;
  int i;
  for (i = 0; i < count; i++) {
    t = max_t * (float)i / (float)(count-1);
    if (curves->evaluate(t, dp._xyz, dp._hpr)) {
      dp._t = t;
      _data.push_back(dp);
    }
  }

  if (curves->get_xyz_curve() != (ParametricCurve *)NULL) {
    _got_xyz = true;
  }
  if (curves->get_hpr_curve() != (ParametricCurve *)NULL) {
    _got_hpr = true;
  }
}



////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::wrap_hpr
//       Access: Public
//  Description: Resets each HPR data point so that the maximum delta
//               between any two consecutive points is 180 degrees,
//               which should prevent incorrect HPR wrapping.
////////////////////////////////////////////////////////////////////
void CurveFitter::
wrap_hpr() {
  Data::iterator di;
  LVecBase3f last(0.0, 0.0, 0.0);
  LVecBase3f net(0.0, 0.0, 0.0);

  for (di = _data.begin(); di != _data.end(); ++di) {
    int i;
    for (i = 0; i < 3; i++) {
      (*di)._hpr[i] += net[i];

      while (((*di)._hpr[i] - last[i]) > 180.0) {
        (*di)._hpr[i] -= 360.0;
        net[i] -= 360.0;
      }

      while (((*di)._hpr[i] - last[i]) < -180.0) {
        (*di)._hpr[i] += 360.0;
        net[i] += 360.0;
      }

      last[i] = (*di)._hpr[i];
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::sort_points
//       Access: Public
//  Description: Sorts all the data points in order by parametric
//               time, in case they were added in an incorrect order.
////////////////////////////////////////////////////////////////////
void CurveFitter::
sort_points() {
  sort(_data.begin(), _data.end());
}

////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::desample
//       Access: Public
//  Description: Removes sample points in order to reduce the
//               complexity of a sampled curve.  Keeps one out of
//               every factor samples.  Also keeps the first and the
//               last samples.
////////////////////////////////////////////////////////////////////
void CurveFitter::
desample(float factor) {
  int in, out;
  float count = factor;

  out = 0;
  for (in = 0; in < (int)_data.size()-1; in++) {
    if (count >= factor) {
      _data[out] = _data[in];
      out++;
      count -= factor;
    }
    count += 1.0;
  }

  _data[out] = _data.back();
  out++;

  _data.erase(_data.begin() + out, _data.end());
}

////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::compute_tangents
//       Access: Public
//  Description: Once a set of points has been built, and prior to
//               calling MakeHermite() or MakeNurbs(),
//               ComputeTangents() must be called to set up the
//               tangents correctly (unless the tangents were defined
//               as the points were added).
////////////////////////////////////////////////////////////////////
void CurveFitter::
compute_tangents(float scale) {
  // If the head and tail points match up, close the curve.
  bool closed = false;

  if (_got_xyz) {
    closed =
      (_data.front()._xyz.almost_equal(_data.back()._xyz, 0.001));

  } else if (_got_hpr) {
    closed =
      (_data.front()._hpr.almost_equal(_data.back()._hpr, 0.001));
  }

  int i;
  int len = _data.size();

  // First, get all the points in the middle, excluding endpoints.
  // These are handled the same whether we are closing the curve or
  // not.
  if (_got_xyz) {
    for (i = 1; i < len-1; i++) {
      _data[i]._tangent =
        (_data[i+1]._xyz - _data[i-1]._xyz) * scale /
        (_data[i+1]._t - _data[i-1]._t);
    }
  }
  if (_got_hpr) {
    for (i = 1; i < len-1; i++) {
      _data[i]._hpr_tangent =
        (_data[i+1]._hpr - _data[i-1]._hpr) * scale /
        (_data[i+1]._t - _data[i-1]._t);
    }
  }

  // Now handle the endpoints.
  if (closed) {
    if (_got_xyz) {
      _data[0]._tangent = _data[len-1]._tangent =
        (_data[1]._xyz - _data[len-2]._xyz) * scale /
        ((_data[1]._t - _data[0]._t) + (_data[len-1]._t - _data[len-2]._t));
    }
    if (_got_hpr) {
      _data[0]._tangent = _data[len-1]._tangent =
        (_data[1]._hpr - _data[len-2]._hpr) * scale /
        ((_data[1]._t - _data[0]._t) + (_data[len-1]._t - _data[len-2]._t));
    }

  } else {
    if (_got_xyz) {
      _data[0]._tangent =
        (_data[1]._xyz - _data[0]._xyz) * scale /
        ((_data[1]._t - _data[0]._t) * 2.0);
      _data[len-1]._tangent =
        (_data[len-1]._xyz - _data[len-2]._xyz) * scale /
        ((_data[len-1]._t - _data[len-2]._t) * 2.0);
    }
    if (_got_hpr) {
      _data[0]._tangent =
        (_data[1]._hpr - _data[0]._hpr) * scale /
        ((_data[1]._t - _data[0]._t) * 2.0);
      _data[len-1]._tangent =
        (_data[len-1]._hpr - _data[len-2]._hpr) * scale /
        ((_data[len-1]._t - _data[len-2]._t) * 2.0);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::make_hermite
//       Access: Public
//  Description: Converts the current set of data points into a
//               Hermite curve.
////////////////////////////////////////////////////////////////////
PT(ParametricCurveCollection) CurveFitter::
make_hermite() const {
  PT(ParametricCurveCollection) result = new ParametricCurveCollection;

  if (_got_xyz) {
    HermiteCurve *hc = new HermiteCurve;
    result->add_curve(hc);
    hc->set_curve_type(PCT_XYZ);

    Data::const_iterator di;
    for (di = _data.begin(); di != _data.end(); ++di) {
      int n = hc->insert_cv((*di)._t);
      hc->set_cv_type(n, HC_SMOOTH);
      hc->set_cv_point(n, (*di)._xyz);
      hc->set_cv_in(n, (*di)._tangent);
      hc->set_cv_out(n, (*di)._tangent);
    }
  }

  if (_got_hpr) {
    HermiteCurve *hc = new HermiteCurve;
    result->add_curve(hc);
    hc->set_curve_type(PCT_HPR);

    Data::const_iterator di;
    for (di = _data.begin(); di != _data.end(); ++di) {
      int n = hc->insert_cv((*di)._t);
      hc->set_cv_type(n, HC_SMOOTH);
      hc->set_cv_point(n, (*di)._hpr);
      hc->set_cv_in(n, (*di)._hpr_tangent);
      hc->set_cv_out(n, (*di)._hpr_tangent);
    }
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::make_nurbs
//       Access: Public
//  Description: Converts the current set of data points into a
//               NURBS curve.  This gives a smoother curve than
//               produced by MakeHermite().
////////////////////////////////////////////////////////////////////
PT(ParametricCurveCollection) CurveFitter::
make_nurbs() const {
  // We start with the HermiteCurves produced above, then convert them
  // to NURBS form.
  PT(ParametricCurveCollection) hermites = make_hermite();
  PT(ParametricCurveCollection) result = new ParametricCurveCollection;

  int num_curves = hermites->get_num_curves();
  for (int c = 0; c < num_curves; c++) {
    ClassicNurbsCurve *nc = new ClassicNurbsCurve(*hermites->get_curve(c));
    result->add_curve(nc);

    // Now we even out the knots to smooth out the curve and make
    // everything c2 continuous.

    int num_knots = nc->get_num_knots();

    // We expect this to be a 4th order curve, since we just converted
    // it from a Hermite.
    assert(nc->get_order() == 4);
    assert(num_knots > 0);

    // Now the knot sequence goes something like this:
    //    0 0 0 0 1 1 1 2 2 2 3 3 3 4 4 4 4

    // We'll consider pairs of knot values beginning at position 3 and
    // every third position thereafter.  We just even out these values
    // between their two neighbors.

    int i;
    float k1, k2 = nc->get_knot(num_knots-1);
    for (i = 3; i < num_knots - 4; i += 3) {
      k1 = nc->get_knot(i-1);
      k2 = nc->get_knot(i+2);
      nc->set_knot(i, (k1 + k1 + k2) / 3.0);
      nc->set_knot(i+1, (k1 + k2 + k2) / 3.0);
    }

    // The last knot must have the terminal value.
    nc->set_knot(num_knots-4, k2);

    // Finally, recompute the curve.
    nc->recompute();
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CurveFitter::
output(ostream &out) const {
  out << "CurveFitter, " << _data.size() << " samples.\n";
}

////////////////////////////////////////////////////////////////////
//     Function: CurveFitter::write
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CurveFitter::
write(ostream &out) const {
  out << "CurveFitter, " << _data.size() << " samples:\n";
  Data::const_iterator di;
  for (di = _data.begin(); di != _data.end(); ++di) {
    out << "  " << (*di) << "\n";
  }
}

