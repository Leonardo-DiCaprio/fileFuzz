SET ( LIBRARY_MAIN_SRC
      audioConverter.cpp  
      audioHook.cpp
      hookHandler.cpp
      videoHook.cpp
      oggBOSExtractorFactory.cpp
      streamMux.cpp
      streamSerializer.cpp
      cmdlineextractor.cpp
    )

ADD_LIBRARY ( ovtmain ${LIBRARY_MAIN_SRC} )

