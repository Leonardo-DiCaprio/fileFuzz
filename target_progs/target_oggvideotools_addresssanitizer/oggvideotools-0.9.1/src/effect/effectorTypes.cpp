//
// C++ Implementation: effectortypes
//
// Description:
//
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

#include "effectorTypes.h"

EffectorType GetEffectorType::operator()(const Effector& effector)
{
  effector.accept(*this);
  return t;
}

void GetEffectorType::visit(const KenBurnsEffect&)
{
  t = KenBurns;
}

void GetEffectorType::visit(const Crossfader&)
{
  t = Crossfade;
}

void GetEffectorType::visit(const LowpassEffect&)
{
  t = Blur;
}

void GetEffectorType::visit(const PlainPicture&)
{
  t = Plain;
}

void GetEffectorType::visit(const ShiftEffect&)
{
  t = Shift;
}

void GetEffectorType::visit(const ShiftblendEffect&)
{
  t = ShiftBlend;
}
