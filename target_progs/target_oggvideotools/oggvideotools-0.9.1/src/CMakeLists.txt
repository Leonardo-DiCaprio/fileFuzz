INCLUDE_DIRECTORIES ( ${THEORADEC_INCLUDE_DIRS}
                      ${THEORAENC_INCLUDE_DIRS}
                      ${VORBIS_INCLUDE_DIRS}
                      ${VORBISENC_INCLUDE_DIRS}
                      ${OGG_INCLUDE_DIRS}
                      ${GD_INCLUDE_DIRS}
                      ${CMAKE_CURRENT_LIST_DIR}
                      ${CMAKE_CURRENT_LIST_DIR}/base
                      ${CMAKE_CURRENT_LIST_DIR}/effect
                      ${CMAKE_CURRENT_LIST_DIR}/misc
                      ${CMAKE_CURRENT_LIST_DIR}/ovt_kate
                      ${CMAKE_CURRENT_LIST_DIR}/ovt_vorbis
                      ${CMAKE_CURRENT_LIST_DIR}/ovt_theora
                      ${CMAKE_CURRENT_LIST_DIR}/main
                      ${CMAKE_CURRENT_LIST_DIR}/libresample
)

ADD_SUBDIRECTORY ( base )
ADD_SUBDIRECTORY ( effect )
ADD_SUBDIRECTORY ( binaries )
ADD_SUBDIRECTORY ( misc )
ADD_SUBDIRECTORY ( ovt_kate )
ADD_SUBDIRECTORY ( ovt_vorbis )
ADD_SUBDIRECTORY ( ovt_theora )
ADD_SUBDIRECTORY ( main )
ADD_SUBDIRECTORY ( libresample )


