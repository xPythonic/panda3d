// Filename: geomLine.h
// Created by:  charles (13Jul00)
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

#ifndef GEOMLINE_H
#define GEOMLINE_H

#include "geom.h"

////////////////////////////////////////////////////////////////////
//       Class : GeomLine
// Description : Line Primitive
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GeomLine : public Geom
{
public:
  GeomLine( void ) : Geom() { _width = 1.0; }
  virtual Geom *make_copy() const;
  virtual void print_draw_immediate( void ) const { }
  virtual void draw_immediate(GraphicsStateGuardianBase *gsg) const;

  virtual int get_num_vertices_per_prim() const {
    return 2;
  }
  virtual int get_num_more_vertices_than_components() const {
    return 0;
  }
  virtual bool uses_components() const {
    return false;
  }

  virtual int get_length(int) const {
    return 2;
  }

  virtual Geom *explode() const {
    return new GeomLine(*this);
  }

  INLINE void set_width(float width) { _width = width; }
  INLINE float get_width(void) const { return _width; }

protected:
  float _width;

public:
  static void register_with_read_factory(void);
  virtual void write_datagram(BamWriter* manager, Datagram &me);

  static TypedWritable *make_GeomLine(const FactoryParams &params);

protected:
  void fillin(DatagramIterator& scan, BamReader* manager);

PUBLISHED:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
public:
  static void init_type() {
    Geom::init_type();
    register_type(_type_handle, "GeomLine",
                  Geom::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif
