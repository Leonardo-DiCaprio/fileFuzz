SET ( LIBRARY_BASE_SRC
      oggPage.cpp
      oggPacket.cpp
      oggComment.cpp
      oggDecoder.cpp
      oggEncoder.cpp
      oggRingbuffer.cpp
      oggStreamDecoder.cpp
      oggStreamEncoder.cpp
      mediaUnit.cpp
      mediaRepository.cpp
      mediaDecoder.cpp
      mediaEncoder.cpp
      mediaConverter.cpp
      granulePosInterpreter.cpp
      mediaOutputDecoder.cpp
      mediaInputEncoder.cpp
      streamParameter.cpp
      streamExtractor.cpp
      bufferRepository.cpp
      fileRepository.cpp
      rawMediaPacket.cpp
)

ADD_LIBRARY ( ovtbase ${LIBRARY_BASE_SRC} )

add_subdirectory(test)