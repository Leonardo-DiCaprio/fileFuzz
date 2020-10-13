SET ( LIBRARY_MISC_SRC
      ringbuffer.cpp
      log.cpp
      helper.cpp
      crc.cpp
)

ADD_LIBRARY ( ovtmisc ${LIBRARY_MISC_SRC} )
