/*
 * MediaConverter is the base class for all subsequent decoders
 * and encoders
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

#include <iostream>
#include "mediaConverter.h"
#include "log.h"

MediaConverter::MediaConverter()
  : mediaConverterState(mdec_free)
{
}

MediaConverter::~MediaConverter()
{
}

void MediaConverter::setInitialized()
{
  if (mediaConverterState == mdec_free)
    mediaConverterState = mdec_initialized;
  else
    logger.error() << "MediaConverter::setInitialized(): double initalization\n";
}

void MediaConverter::setConfigured()
{
  if (mediaConverterState < mdec_configured)
    mediaConverterState = mdec_configured;
  else
    logger.error() << "MediaConverter::setConfigured(): decoder is configured\n";
}

void MediaConverter::setAvailable()
{
  if ((mediaConverterState >= mdec_configured) &&
      (mediaConverterState <= mdec_available))
    mediaConverterState = mdec_available;
  else
    logger.error() << "MediaConverter::setAvailable(): decoder is not configured or has ended\n";
}

void MediaConverter::setEmpty()
{
  if ((mediaConverterState == mdec_configured) ||
      (mediaConverterState == mdec_available) ||
      (mediaConverterState == mdec_empty))
    mediaConverterState = mdec_empty;
  else
    logger.error() << "MediaConverter::setEmpty(): decoder not configured correctly\n";
}

void MediaConverter::setEndOfStream()
{
  if (mediaConverterState > mdec_initialized)
    mediaConverterState = mdec_endofStream;
  else
    logger.error() << "MediaConverter::setEndOfStream(): decoder not initialized\n";
}

void MediaConverter::setFree()
{
  mediaConverterState = mdec_free;
}

bool MediaConverter::isInitialized()
{
  return(mediaConverterState > mdec_free);
}

bool MediaConverter::isConfigured()
{
  return(mediaConverterState > mdec_initialized);
}

bool MediaConverter::isAvailable()
{
  return(mediaConverterState == mdec_available);
}

bool MediaConverter::isEmpty()
{
  return( (mediaConverterState == mdec_initialized) ||
          (mediaConverterState == mdec_empty) ||
          (mediaConverterState == mdec_configured));
}

bool MediaConverter::isEndOfStream()
{
  return(mediaConverterState == mdec_endofStream);
}
