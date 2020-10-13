/*
 * RefObject class to reduce complete object copy actions
 *
 * Copyright (C) 2005-2008 Joern Seger
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

#ifndef refObject_h
#define refObject_h

//! class to keep refences instead of copies
/*! This class keeps track of an dynamically allocated
    memory on the heap.
    On object copy, only the object pointer is copied and
    an additionally reference counter is incremented.
    On object elemination, the object is deleted only in
    case that no one has still a refence to this object.
    (Code is inspired by Bjarne Stoustrup)

    This code is obsoleted by the boost smart pointers,
    however, actually I do not plan to create a dependency
    to boost
*/
template<class C>
class RefObject {

protected:

  unsigned int* refCounter;
  C* objPtr;

public:
  RefObject();
  RefObject(C* objPtr);
  RefObject(const RefObject& obj);
  virtual ~RefObject();

  RefObject& operator=(const RefObject& obj);

  C* operator->();

  C* obj();
  void obj(C* object);

};

/* Implementation Part */

template<class C> inline RefObject<C>::RefObject()
  : refCounter(new unsigned int), objPtr(new C)
{
  (*refCounter) = 1;
}

template<class C> inline RefObject<C>::RefObject(C* objPtr)
  : refCounter(new unsigned int), objPtr(objPtr)
{
  (*refCounter) = 1;
}

template<class C> inline RefObject<C>::RefObject(const RefObject& refObj)
  : refCounter(refObj.refCounter), objPtr(refObj.objPtr)
{
  if (this == &refObj)
    return;
  (*refCounter)++;
}

template<class C> inline RefObject<C>::~RefObject()
{
  (*refCounter)--;

  if ((*refCounter) == 0) {
    delete refCounter;
    delete objPtr;
  }
}

template<class C> inline RefObject<C>&
RefObject<C>::operator=(const RefObject& refObj)
{
  if (this == &refObj)
    return(*this);

  (*refCounter)--;

  if ((*refCounter) == 0) {
    delete refCounter;
    delete objPtr;
  }

  refCounter = refObj.refCounter;
  objPtr     = refObj.objPtr;

  (*refCounter)++;
  return(*this);
}

template<class C> inline C* RefObject<C>::obj()
{
  return(objPtr);
}

template<class C> inline C* RefObject<C>::operator->()
{
  return(objPtr);
}

template<class C> inline void RefObject<C>::obj(C* ptr)
{
  delete objPtr;
  objPtr = ptr;

  /* reference pointer is not touched
     DANGER: think - do you really want to use this method? */
}

#endif // refObject_h
