// Filename: executionEnvironment.h
// Created by:  drose (15May00)
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

#ifndef EXECUTIONENVIRONMENT_H
#define EXECUTIONENVIRONMENT_H

#include <dtoolbase.h>

#include "vector_string.h"
#include "filename.h"

#include <map>

////////////////////////////////////////////////////////////////////
//       Class : ExecutionEnvironment
// Description : Encapsulates access to the environment variables and
//               command-line arguments at the time of execution.
//               This is encapsulated to support accessing these
//               things during static init time, which seems to be
//               risky at best.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOL ExecutionEnvironment {
private:
  ExecutionEnvironment();

public:
  INLINE static bool has_environment_variable(const string &var);
  INLINE static string get_environment_variable(const string &var);

  static string expand_string(const string &str);

  INLINE static int get_num_args();
  INLINE static string get_arg(int n);

  INLINE static string get_binary_name();

  static Filename get_cwd();

private:
  bool ns_has_environment_variable(const string &var) const;
  string ns_get_environment_variable(const string &var) const;

  int ns_get_num_args() const;
  string ns_get_arg(int n) const;

  string ns_get_binary_name() const;

  static ExecutionEnvironment *get_ptr();

  void read_environment_variables();
  void read_args();

private:
  typedef map<string, string> EnvironmentVariables;
  EnvironmentVariables _variables;

  typedef vector_string CommandArguments;
  CommandArguments _args;

  string _binary_name;

  static ExecutionEnvironment *_global_ptr;
};

#include "executionEnvironment.I"

#endif
