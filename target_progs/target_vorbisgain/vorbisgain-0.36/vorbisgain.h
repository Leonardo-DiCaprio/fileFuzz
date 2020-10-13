#ifndef VG_VORBISGAIN_H
#define VG_VORBISGAIN_H

/** Information about a file to process */
typedef struct file_list
{
    struct file_list* next_file;
    const char* filename;
    float track_gain;
    float track_peak;
    float album_gain;       /**< File's ablum gain value; only used when
                                 converting tags */
    int skip;               /**< Don't process this file (already tagged or
                                 not a Vorbis file) */
} FILE_LIST;


/** Settings and misc other global data */
typedef struct settings
{
    FILE_LIST* file_list;   /**< Files to process (possibly as an album) */
#ifdef ENABLE_RECURSIVE
    char* pattern;          /**< Pattern to match file names against */
#endif
    float album_gain;       /**< Album gain specified on the command line */
    int album_gain_set;     /**< Album gain has been set on the command line.
                                 Don't calculate it */
    int first_file;         /**< About to process the first file in an album */
    int album;              /**< Calculate album gain values as well */
    int clean;              /**< Remove all replay gain tags */
    int convert;            /**< Convert old format tags to new format */
    int display_only;
    int fast;               /**< Skip files that already have all needed tags */
    int quiet;
#ifdef ENABLE_RECURSIVE
    int recursive;
#endif
    int skip;               /**< Skip non-vorbis files */
    int show_progress;
} SETTINGS;


extern int  add_to_list(FILE_LIST** list, const char* file);
extern void free_list(FILE_LIST* list);
extern int  process_files(FILE_LIST* file_list, SETTINGS* settings);

#endif /* VG_VORBISGAIN_H */
