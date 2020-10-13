//
// C++ Interface: effectortypes
//
// Description:
// Enumeration of effector types and a a specialized EffectorVisitor that
// allows for a more effective effector type detection than dynamic_cast<>.
//
// Copyright (C) 2010  Bjarne Juul Pasgaard <bjvest@users.sourceforge.net>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
// USA.

#ifndef EFFECTORTYPES_H
#define EFFECTORTYPES_H

#include "effector.h"
#include "effectorVisitor.h"

/// @brief Enumeration of the available effector types.
enum EffectorType {
  KenBurns,   ///< Ken Burns effect
  Crossfade,  ///< Cross fading
  Plain,      ///< Plain pictures
  Blur,       ///< Bluring at changeover
  Shift,      ///< Shift left effect
  ShiftBlend  ///< Shift and blend left effect
};

// Forward declarations
class KenBurnsEffect;
class Crossfader;
class LowpassEffect;
class PlainPicture;
class ShiftEffect;
class ShiftblendEffect;

/// @brief A functor that determines the type of an
///        effector specialization.
///
/// This is an alternative to dynamic_cast<>, but is often
/// far more effective and has the advantage of a known
/// constant-time complexity (in contrast to dynamic_cast<>).
class GetEffectorType : protected EffectorVisitor {
public:

  /// @brief The entry point of the functor.
  ///
  /// @param effector The effector to determine the type of.
  ///
  /// @return The type of the supplied effector.
  EffectorType operator()(const Effector& effector);

  virtual ~GetEffectorType() {}

protected:

  // Overridden base class methods
  virtual void visit(const KenBurnsEffect&);
  virtual void visit(const Crossfader&);
  virtual void visit(const LowpassEffect&);
  virtual void visit(const PlainPicture&);
  virtual void visit(const ShiftEffect&);
  virtual void visit(const ShiftblendEffect&);

protected:

  EffectorType t;

};

#endif
