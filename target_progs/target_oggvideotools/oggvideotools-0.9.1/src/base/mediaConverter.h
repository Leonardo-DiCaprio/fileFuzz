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

#ifndef MEDIACONVERTER_H_
#define MEDIACONVERTER_H_

#include <string>

//! MediaConverter is a baseclass for all decoders
/*! The MediaConverter class is used with a defined decoder or encoder. It mainly implements a state machine,
 which holds the actual decoder/encoder state information:
 #- mdec_free: the decoder is uninitialized
 #- mdec_initialized: the decoder is initialized, i.e. the stream has a defined stream type or stream ID
 #- mdec_configured: the stream is able to give information about the stream, that is processed (stream header is read)
 #- mdec_available: the stream is able to provide output data (e.g. a video picture etc.)
 #- mdec_empty: there is actually no data available for output
 #- mdec_endofStream: the end of a stream has been detected
 */
class MediaConverter {
protected:
  enum MediaConverterState {
    mdec_free,
    mdec_initialized,
    mdec_configured,
    mdec_empty,
    mdec_available,
    mdec_endofStream
  };

private:
  MediaConverterState mediaConverterState;

protected:

  void setInitialized();
  void setConfigured();
  void setAvailable();
  void setEmpty();
  void setFree();

public:
  MediaConverter();
  virtual ~MediaConverter();

  void setEndOfStream();

  bool isInitialized();
  bool isConfigured();
  bool isAvailable();
  bool isEmpty();
  bool isEndOfStream();

  virtual std::string configuration() const {
    return (std::string(""));
  }

  virtual void reset() {}
};

#endif /*MEDIACONVERTER_H_*/
