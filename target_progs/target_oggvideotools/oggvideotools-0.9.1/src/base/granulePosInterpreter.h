#ifndef GRANULEPOSINTERPRETER_H_
#define GRANULEPOSINTERPRETER_H_

#include "oggPacket.h"
#include "streamParameter.h"

//! Class to interprete the granule position of a m_oggPacket
/*! Every m_oggPacket carries a granule position within its header information.
 * This granule position is a 64 Bit value and is interpreted quite different
 * across the available ogg encoders/decoders.
 *
 * To be able to convert a 64 Bit number into a time position this interpreter
 * needs some more information, which are given by the initialize() method.
 *
 * E.g. an vorbis interpretes the granule position as the audio samples. If
 * an audio file has been sample with a rate of 16 kHz, the position 0 is the
 * first sample at time 0 ms. The granule Position 1000 would be a time position
 * of 62.5 ms. At a sample rate of 32 kHz this would be the time position 31.25 ms.
 * So in this case we need to know the sample rate as a basis for the time calculation.
 *
 * These decoder specific information are given by the corresponding class object
 * derived from the abstract StreamParameter.
 *
 * This class can interpret a given granule position and it is able to caclulate
 * new positions by given ones (additions of two positions)
*/
class GranulePosInterpreter {

protected:
  bool initialized;

  int64 actualGranulePosition;

public:
  GranulePosInterpreter();
  virtual ~GranulePosInterpreter();

  //! method to initialize the interpreter
  /*! The initialization is needed to be able to
   *  interpret the granule position. E.g. theora splits the
   *  64 Bit into a lower and upper part for the last keyframe
   *  position and a counter for the p-frames. This split
   *  is depending on the header information in the first page.
   */
  virtual void initialize(StreamParameter* parameter) = 0;

  //! method to interpret the time from a granule position
  /*! This method interprets the given granule position by the
   *  information from the first page, with which the interpreter has
   *  been initialized. The time does not depend on the actual
   *  internal position counter.
   *  @param granulePos granuale position, that should be interpreted with the implicite information from the interpreter
   *  @return the time of the position given by the parameter*/
  virtual double getTime(int64 granulePos) = 0;

  //! Method to interprete the internal position to a time
  /*! This method uses the actual internal position information
   *  to calculate the actual time. For the interpretation the method
   *  uses the information given by the first page.
   *  */
  double getActTime();

  //! method to return the actual granule position
  /*! @return returns the actual/calculated granule position
   * */
  virtual int64 getPosition();

  //! Method places the actual stream position into an m_oggPacket.
  /*! @param packet OggPacket to replace the granule position field in the header
   * */
  virtual void setStreamPosition(OggPacket& packet) = 0;

  //! Method to add a second position to this position
  /*! @param position that should be added to the actual (implicite) position of this object */
//    virtual GranulePosInterpreter& operator+=(GranulePosInterpreter& position) = 0;

  //! Method to substract a second position from this position
  /*! @param position that should be substracted from the actual (implicite) position of this object */
//    virtual GranulePosInterpreter& operator-=(GranulePosInterpreter& position) = 0;

};

#endif /*GRANULEPOSINTERPRETER_H_*/
