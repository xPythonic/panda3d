// Filename: baseParticle.h
// Created by:  charles (14Jun00)
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

#ifndef BASEPARTICLE_H
#define BASEPARTICLE_H

#include <pandabase.h>
#include <physicsObject.h>

////////////////////////////////////////////////////////////////////
//       Class : BaseParticle
// Description : An individual, physically-modelable particle
//               abstract base class.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS BaseParticle : public PhysicsObject {
private:
  // NOTE: age and lifespan are in seconds.
  float _age;
  float _lifespan;
  bool _alive;

  LPoint3f _last_position;

protected:
  BaseParticle(int lifespan = 1, bool alive = false);
  BaseParticle(const BaseParticle &copy);
  virtual ~BaseParticle(void);

public:

  // local methods
  INLINE void set_age(float age);
  INLINE void set_lifespan(float lifespan);
  INLINE void set_alive(bool alive);

  INLINE float get_age(void) const;
  INLINE float get_lifespan(void) const;
  INLINE bool get_alive(void) const;

  INLINE float get_parameterized_age(void) const;
  INLINE float get_parameterized_vel(void) const;

  // child methods
  virtual void init(void) = 0;
  virtual void die(void) = 0;
  virtual void update(void) = 0;

  // for spriteParticleRenderer
  virtual float get_theta(void) const;

  // from PhysicsObject
  virtual PhysicsObject *make_copy(void) const = 0;
};

#include "baseParticle.I"

#endif // BASEPARTICLE_H
