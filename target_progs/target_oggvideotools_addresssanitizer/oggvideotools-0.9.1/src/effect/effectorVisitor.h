//
// C++ Interface: effectorvisitor
//
// Description:
// A visitor (in accordance with the visitor design patter) for
// specializations of the Effector class.
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

#ifndef EFFECTORVISITOR_H
#define EFFECTORVISITOR_H

// Forward declarations
class KenBurnsEffect;
class Crossfader;
class LowpassEffect;
class PlainPicture;
class ShiftEffect;
class ShiftblendEffect;

/// @brief A visitor of Effector specialisations
///
/// This class takes the role of a visitor in accordance with the
/// visitor design pattern.
class EffectorVisitor {
public:

  virtual ~EffectorVisitor() {};

  virtual void visit(const KenBurnsEffect&);
  virtual void visit(const Crossfader&);
  virtual void visit(const LowpassEffect&);
  virtual void visit(const PlainPicture&);
  virtual void visit(const ShiftEffect&);
  virtual void visit(const ShiftblendEffect&);

};

#endif
