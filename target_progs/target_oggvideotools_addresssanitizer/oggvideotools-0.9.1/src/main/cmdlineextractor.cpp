//
// C++ Implementation: cmdlineextractor
//
// Description:
//
//
// Author: Yorn <yorn@gmx.net>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "cmdlineextractor.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <sstream>

#include "definition.h"
#include "oggComment.h"
#include "pictureLoader.h"
#include "log.h"
#include "effectorTypes.h"

CmdlineExtractor::CmdlineExtractor()
{
}

CmdlineExtractor::~CmdlineExtractor()
{
}

void CmdlineExtractor::extractCommentPairs(std::vector<OggComment>& list,
    const std::string& _argument, char tokenSeparator,
    char commentSeparator)
{
  std::string argument(_argument);
  std::stringstream str;
  std::string substr;

  // delete all invalid data
  std::size_t pos;
  while ((pos = argument.find_first_not_of(validTextChars))
         != std::string::npos) {
#ifdef DEBUG
    logger.debug() << "Erasing sign <"<<argument.at(pos) <<"> - it is invalid\n";
#endif
    argument.erase(pos, 1);
  }

  // if there is no argument given, the first frame will be created as a thumbnail
  if (argument.empty()) {
    return;
  }

  str << argument;

  while (!str.eof()) {
    getline(str, substr, tokenSeparator);

    std::size_t commentSeparatorPos;
    if ((commentSeparatorPos = substr.find_first_of(commentSeparator))
        != std::string::npos) {
      OggComment comment;
      comment.tag = substr.substr(0, commentSeparatorPos);
      comment.value = substr.substr(commentSeparatorPos + 1,
                                    std::string::npos);
      list.push_back(comment);
      //			logger.debug() << "Found pair "<<comment.tag<<" "<<comment.value
      //			<<std::endl;
    }

  }

}

void CmdlineExtractor::extractUint32(std::deque<uint32>& list,
                                     const std::string& _argument, char seperator)
{
  std::string argument(_argument);
  std::stringstream str;
  std::string substr;

  // delete all invalid data
  std::size_t pos;
  while ((pos = argument.find_first_not_of(validChars)) != std::string::npos) {
#ifdef DEBUG
    logger.debug() << "erasing <"<<argument.at(pos) <<">\n";
#endif
    argument.erase(pos, 1);
  }

  // if there is no argument given, the first frame will be created as a thumbnail
  if (argument.empty()) {
    list.push_back(0);
    return;
  }

  str << argument;

  uint32 value(0);
  while (!str.eof()) {
    std::stringstream part;
    getline(str, substr, seperator);
    part << substr;
    part >> value;
    list.push_back(value);
  }

}

void CmdlineExtractor::extractBlend(std::vector<BlendElement>& list,
                                    const std::string& _argument, char tokenSeparator,
                                    char valueSeparator)
{

  std::string argument(_argument);
  std::stringstream str;
  std::string substr;

  // delete all invalid data
  std::size_t pos;
  while ((pos = argument.find_first_not_of(validTextChars))
         != std::string::npos) {
    argument.erase(pos, 1);
  }

  // if there is no argument given, the first frame will be created as a thumbnail
  if (argument.empty()) {
    return;
  }

  str << argument;

  while (!str.eof()) {
    getline(str, substr, tokenSeparator);

    /* extract picture name */
    std::size_t valueSeparatorPos = substr.find_first_of(valueSeparator);

    std::string filename = substr.substr(0, valueSeparatorPos);

    /* extract all extra data if some (start time, end time, smoothing)*/
    double startTime(0);
    double endTime(-1);
    bool smooth(false);
    std::stringstream tmp;

    /* are there any other information given? */
    if (valueSeparatorPos != std::string::npos) {

      /* analysing start time */
      substr = substr.substr(valueSeparatorPos + 1);

      valueSeparatorPos = substr.find_first_of(valueSeparator);

      tmp << substr.substr(0, valueSeparatorPos);
      tmp >> startTime;
      tmp.clear();

      if (valueSeparatorPos != std::string::npos) {

        /* analysing start time */
        substr = substr.substr(valueSeparatorPos + 1);

        valueSeparatorPos = substr.find_first_of(valueSeparator);

        tmp << substr.substr(0, valueSeparatorPos);
        tmp >> endTime;

        if (valueSeparatorPos != std::string::npos) {

          /* analysing start time */
          substr = substr.substr(valueSeparatorPos + 1);

          if (substr.substr(0, valueSeparator) == "s")
            smooth = true;
        }
      }

    }

    BlendElement elem(filename, startTime, endTime, smooth);
    //    elem.loadPicture();
    list.push_back(elem);

  }

#ifdef DEBUG
  for (uint32 i( 0); i<list.size(); ++i) {
    logger.debug() << "Info: picture"<<i<<": startTime="<<list[i].startTime
                   <<" endTime="<<list[i].endTime<<" smooth=";
    if (list[i].smooth == true)
      logger.debug() << "true\n";
    else
      logger.debug() << "false\n";

  }
#endif
}

uint32 CmdlineExtractor::atoi(const std::string& _argument)
{
  std::stringstream stream;
  uint32 value;

  stream << _argument;
  stream >> value;

  return (value);

}

float CmdlineExtractor::atof(const std::string& _argument)
{
  std::stringstream stream;
  float value;

  stream << _argument;
  stream >> value;

  return (value);

}

uint32 CmdlineExtractor::getNextUint32(std::string& argument,
                                       char tokenSeparator)
{
  uint32 retValue(0);

  if (!argument.empty()) {

    std::stringstream tmp;

    std::size_t tokenPosition(argument.find_first_of(tokenSeparator));
    tmp << argument.substr(0, tokenPosition);
    tmp >> retValue;

    argument = argument.substr(tokenPosition + 1);

  }

  return (retValue);
}

float CmdlineExtractor::getNextFloat(std::string& argument,
                                     char tokenSeparator)
{
  float retValue(0.0);

  if (!argument.empty()) {

    std::stringstream tmp;

    std::size_t tokenPosition(argument.find_first_of(tokenSeparator));
    tmp << argument.substr(0, tokenPosition);
    tmp >> retValue;

    argument = argument.substr(tokenPosition + 1);

  }

  return (retValue);
}

std::string CmdlineExtractor::getNextString(std::string& argument,
    char tokenSeparator)
{
  std::string retValue(0);

  if (!argument.empty()) {

    std::stringstream tmp;

    std::size_t tokenPosition(argument.find_first_of(tokenSeparator));
    tmp << argument.substr(0, tokenPosition);
    tmp >> retValue;

    argument = argument.substr(tokenPosition + 1);

  }

  return (retValue);
}

void CmdlineExtractor::extractSlideshow(const std::string& _argument,
                                        char tokenSeparator, SlideshowElement& slideshowElement)
{
  /* A full specified picture would look like this (speparator is ",")
   * name.jpg:<duration>,<type>,<typeSpecificData>
   * This should go into a creator factory later:
   * start and end position is written as <X-Position>,<YPosition>,<Zoom>
   * The X and Y Position is from the left upper corner. The Zoom is 1 if
   * the pixel is just copy. In that case, the subframe is as big is the
   * outgoing frame */

  std::string argument(_argument);
  std::stringstream tmp;

  // delete all invalid data
  std::size_t pos;
  while ((pos = argument.find_first_not_of(validTextChars))
         != std::string::npos) {
    argument.erase(pos, 1);
  }

  // if there is no argument given, the first frame will be created as a thumbnail
  if (argument.empty()) {
    return;
  }

  /* extract picture name */
  std::size_t tokenPosition(argument.find_first_of(tokenSeparator));
  slideshowElement.filename = argument.substr(0, tokenPosition);

  std::string substr = argument.substr(tokenPosition + 1);

  /* extract length */
//	if (tokenPosition != std::string::npos) {
//		std::string substr = argument.substr(tokenPosition + 1);
//		tokenPosition = substr.find_first_of(tokenSeparator);
//		tmp << substr.substr(0, tokenPosition);
//		tmp >> slideshowElement.duration;
//		substr = substr.substr(tokenPosition + 1);
//		std::cout << substr << std::endl;

//		if (tokenPosition != std::string::npos) {
//			tokenPosition = substr.find_first_of(tokenSeparator);
//			std::string typeName(substr.substr(0, tokenPosition));
//			std::cout << substr << std::endl;
//
//			if ((typeName == "kb") || (typeName == "KB")
//					|| (typeName == "KenBurns") || (typeName == "kenburns"))
//				slideshowElement.type = KenBurns;
//
//			if ((typeName == "p") || (typeName == "pl") || (typeName == "plain")
//					|| (typeName == "Plain"))
//				slideshowElement.type = Plain;
//
//			if ((typeName == "cf") || (typeName == "crossfade")
//					|| (typeName == "CF") || (typeName == "Crossfade"))
//				slideshowElement.type = Crossfade;
//
//			if ((typeName == "bl") || (typeName == "b") || (typeName == "B")
//					|| (typeName == "blur") || (typeName == "bluring"))
//				slideshowElement.type = Blur;

  substr = substr.substr(tokenPosition + 1);

  if (tokenPosition != std::string::npos) {
    std::cout << "F " << substr << std::endl;

    slideshowElement.startPosX = getNextUint32(substr,
                                 tokenSeparator);
    std::cout << substr << std::endl;
    slideshowElement.startPosY = getNextUint32(substr,
                                 tokenSeparator);
    std::cout << substr << std::endl;
    slideshowElement.startZoom = getNextFloat(substr,
                                 tokenSeparator);
    std::cout << substr << std::endl;
    slideshowElement.endPosX = getNextUint32(substr,
                               tokenSeparator);
    std::cout << substr << std::endl;
    slideshowElement.endPosY = getNextUint32(substr,
                               tokenSeparator);
    std::cout << substr << std::endl;
    slideshowElement.endZoom = getNextFloat(substr, tokenSeparator);
  }
//		}
  //}
}

void CmdlineExtractor::extractCrossSequence(std::vector<std::string>& list,
    const std::string& _argument, char tokenSeparator)
{
  std::string argument(_argument);

  // delete all invalid data
  std::size_t pos;
  while ((pos = argument.find_first_not_of(validTextChars))
         != std::string::npos) {
    argument.erase(pos, 1);
  }

  while (!argument.empty())
    list.push_back(getNextString(argument, tokenSeparator));

  return;
}

