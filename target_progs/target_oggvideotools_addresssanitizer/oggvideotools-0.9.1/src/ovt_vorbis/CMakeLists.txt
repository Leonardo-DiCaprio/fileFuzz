SET ( LIBRARY_VORBIS_SRC
      vorbisDecoder.cpp  
      vorbisEncoder.cpp  
      vorbisExtractor.cpp  
      vorbisPosInterpreter.cpp  
      vorbisStreamParameter.cpp
      audioPacket.cpp
    )

ADD_LIBRARY ( ovtvorbis STATIC ${LIBRARY_VORBIS_SRC} )

