SET ( LIBRARY_THEORA_SRC
      theoraEncoder.cpp
      theoraDecoder.cpp  
      theoraExtractor.cpp  
      theoraPosInterpreter.cpp
      theoraStreamParameter.cpp
    )
#      theoraConfig.cpp

ADD_LIBRARY ( ovttheora ${LIBRARY_THEORA_SRC} )

