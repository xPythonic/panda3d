// Filename: guiSign.h
// Created by:  cary (06Nov00)
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

#ifndef __GUISIGN_H__
#define __GUISIGN_H__

#include "guiItem.h"
#include "guiLabel.h"
#include "guiManager.h"

class EXPCL_PANDA GuiSign : public GuiItem {
private:
  PT(GuiLabel) _sign;
  float _sign_scale;

  INLINE GuiSign(void);
  virtual void recompute_frame(void);

PUBLISHED:
  GuiSign(const string&, GuiLabel*);
  ~GuiSign(void);

  virtual void manage(GuiManager*, EventHandler&);
  virtual void manage(GuiManager*, EventHandler&, Node*);
  virtual void unmanage(void);

  virtual int freeze();
  virtual int thaw();

  virtual void set_scale(float);
  virtual void set_scale(float, float, float);
  virtual void set_pos(const LVector3f&);
  virtual void set_priority(GuiLabel*, const Priority);
  virtual void set_priority(GuiItem*, const Priority);

  virtual int set_draw_order(int);

  virtual void output(ostream&) const;

public:
  // type interface
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    GuiItem::init_type();
    register_type(_type_handle, "GuiSign",
                  GuiItem::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type(void) {
    init_type();
    return get_class_type();
  }
private:
  static TypeHandle _type_handle;
};

#include "guiSign.I"

#endif /* __GUISIGN_H__ */
