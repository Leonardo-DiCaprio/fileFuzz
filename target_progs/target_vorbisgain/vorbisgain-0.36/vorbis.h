#ifndef VG_VORBIS_H
#define VG_VORBIS_H

#include "vorbisgain.h"

#define NO_PEAK -1.f
#define NO_GAIN -10000.f

extern int has_tags(const char *filename, int album, int has_any, 
    SETTINGS *settings);
extern int get_old_tags(const char* filename, int album_tag, 
    float* track_gain, float* track_peak, float* album_gain, 
    SETTINGS* settings);
extern int get_gain(const char *filename, float *peak, float *gain, 
    SETTINGS *settings);
extern int write_gains(const char *filename, float track_peak, 
    float track_gain, float album_gain, float album_peak, int verbose, 
    int remove_tags);

#endif /* VG_VORBIS_H */
