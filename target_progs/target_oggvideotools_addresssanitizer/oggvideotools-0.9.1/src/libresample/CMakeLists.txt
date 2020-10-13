SET ( LIBRARY_RESAMPLE_SRC
      filterkit.c  
      resample.c  
      resamplesubs.c
    )

ADD_LIBRARY ( resample ${LIBRARY_RESAMPLE_SRC} )

