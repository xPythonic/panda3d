// Filename: pnmReader.h
// Created by:  drose (14Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef PNMREADER_H
#define PNMREADER_H

#include "pandabase.h"

#include "pnmImageHeader.h"


////////////////////////////////////////////////////////////////////
//       Class : PNMReader
// Description : This is an abstract base class that defines the
//               interface for reading image files of various types.
//               Any particular image file type that can be read must
//               define a class that inherits from PNMReader to read
//               it.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PNMReader : public PNMImageHeader {
protected:
  INLINE PNMReader(PNMFileType *type, istream *file, bool owns_file);

public:
  virtual ~PNMReader();
  INLINE void set_read_size(int x_size, int y_size);

  INLINE PNMFileType *get_type() const;

  virtual void prepare_read();
  virtual int read_data(xel *array, xelval *alpha);
  virtual bool supports_read_row() const;
  virtual bool read_row(xel *array, xelval *alpha, int x_size, int y_size);

  virtual bool supports_stream_read() const;

  INLINE bool is_valid() const;

private:
  int get_reduction_shift(int orig_size, int new_size);

protected:
  PNMFileType *_type;
  bool _owns_file;
  istream *_file;
  bool _is_valid;

  int _read_x_size, _read_y_size;
  bool _has_read_size;

  int _x_shift, _y_shift;
  int _orig_x_size, _orig_y_size;
};

#include "pnmReader.I"

#endif
