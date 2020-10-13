#ifndef STREAMPARAMETER_H_
#define STREAMPARAMETER_H_

#include <string>

//! Pure virtual class to be derived to carry stream specific information
/*! This class has been implemented to define
 *  a number of virtual methods to clone, compare and print out
 *  the stream parameters.
 *
 *  A (derived) stream parameter carries stream specific information, that
 *  is needed e.g. to interpret the granule position of an ogg page.
 * */
class StreamParameter {

public:

  StreamParameter();
  virtual ~StreamParameter();

  //! Method to compare stream parameters
  /*! @param param is the second StreamParameter object, which is compared to this StreamParameter object.
   *  @return true, if both stream parameters match, false if not. */
  virtual bool operator==(const StreamParameter& param) = 0;

  //! Method to create a readable string from this StreamParameter object
  virtual std::string toString() = 0;

  //! Method to clone this StreamParameter object
  virtual StreamParameter* clone() = 0;

};

#endif /*STREAMPARAMETER_H_*/
