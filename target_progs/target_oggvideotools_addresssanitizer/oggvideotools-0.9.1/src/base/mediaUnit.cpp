/*
 * MediaUnit is a baseclass for all media transfer units
 *
 * Copyright (C) 2008 Joern Seger
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "mediaUnit.h"

#include <iostream>

MediaUnit::MediaUnit(MediaDirection_t type, const std::string _name)
  : name(_name), mediaDirection(type)
{
}

MediaUnit::~MediaUnit()
{
}

/*
void MediaUnit::setConfig(MediaConfig* _config)
{
  config = _config->clone();
}

MediaConfig* MediaUnit::getConfig()
{
  return(config->clone());
}
*/
