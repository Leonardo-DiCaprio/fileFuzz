SET ( MAN_SRC oggTranscode.1 oggSlideshow.1 oggThumb.1 oggSplit.1 oggJoin.1
  oggCut.1 oggCat.1 oggSilence.1 oggDump.1 oggLength.1 mkThumbs.1 )
SET ( HTML_SRC oggTranscode.html oggSlideshow.html oggThumb.html oggSplit.html oggJoin.html
  oggCut.html oggCat.html oggSilence.html oggDump.html oggLength.html mkThumbs.html )


IF ( $ENV{MAKE_PACKAGE} )
 INSTALL ( FILES ${MAN_SRC} DESTINATION doc )
 INSTALL ( FILES ${HTML_SRC} DESTINATION doc )
ELSE ( $ENV{MAKE_PACKAGE} )
INSTALL ( FILES ${MAN_SRC} DESTINATION man/man1 )
ENDIF ( $ENV{MAKE_PACKAGE} )
