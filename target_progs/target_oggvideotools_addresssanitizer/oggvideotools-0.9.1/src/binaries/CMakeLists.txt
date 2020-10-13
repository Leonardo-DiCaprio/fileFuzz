SET ( EXECUTABLES oggSplit oggDump oggJoin oggCut oggLength oggCat oggSilence )

SET ( EXEC_WITH_GD oggSlideshow oggThumb oggTranscode )

FOREACH ( exec ${EXECUTABLES} )
  ADD_EXECUTABLE ( ${exec} ${exec}.cpp )
 TARGET_LINK_LIBRARIES ( ${exec} ovtmain ovtkate ovtvorbis ovttheora ovtbase ovteffect ovtmisc resample ${ALL_LIBS} )
ENDFOREACH ( exec ${EXECUTABLES} )
INSTALL ( TARGETS ${EXECUTABLES} DESTINATION bin )

IF ( HAVE_LIBGD )
FOREACH ( exec ${EXEC_WITH_GD} )
  ADD_EXECUTABLE ( ${exec} ${exec}.cpp )
  TARGET_LINK_LIBRARIES ( ${exec} ovtmain ovtkate ovtvorbis ovttheora ovtbase ovteffect ovtmisc resample ${ALL_LIBS}  )
ENDFOREACH ( exec ${EXEC_WITH_GD} )
INSTALL ( TARGETS ${EXEC_WITH_GD} DESTINATION bin )
ENDIF ( HAVE_LIBGD )
