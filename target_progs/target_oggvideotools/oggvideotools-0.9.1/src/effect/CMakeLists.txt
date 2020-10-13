SET ( LIBRARY_VIDEOEFFECT_SRC
      effector.cpp
      effectorTypes.cpp
      effectorVisitor.cpp
      crossfader.cpp
      plainPicture.cpp
      lowpassEffect.cpp
      kenburnseffect.cpp
      pictureResize.cpp
      pictureBlend.cpp
      pictureLoader.cpp
      shiftEffect.cpp
      shiftblendEffect.cpp
      basePlane.cpp
      rgbPlane.cpp
      blendElement.cpp )

ADD_LIBRARY ( ovteffect ${LIBRARY_VIDEOEFFECT_SRC} )
