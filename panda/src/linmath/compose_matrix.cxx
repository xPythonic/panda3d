// Filename: compose_matrix.cxx
// Created by:  drose (27Jan99)
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

#include "deg_2_rad.h"
#include "config_linmath.h"
#include "compose_matrix.h"

Configure(config_linmath_2);
ConfigureFn(config_linmath_2) {
}


static const bool temp_hpr_fix = config_linmath_2.GetBool("temp-hpr-fix", false);

#include "fltnames.h"
#include "compose_matrix_src.cxx"

#include "dblnames.h"
#include "compose_matrix_src.cxx"



