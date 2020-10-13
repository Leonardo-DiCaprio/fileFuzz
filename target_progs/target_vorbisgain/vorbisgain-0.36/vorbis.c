/*
 * Handles all Vorbis-specific ReplayGain processing.
 *
 * This program is distributed under the GNU General Public License, version
 * 2.1. A copy of this license is included with this source.
 *
 * Copyright (C) 2002, 2004 Gian-Carlo Pascutto and Magnus Holmgren
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>
#include "gain_analysis.h"
#include "i18n.h"
#include "misc.h"
#include "vcedit.h"
#include "vorbis.h"

#include <sys/stat.h>
#ifdef WIN32
#include <io.h>
#include <sys/utime.h>
#else /* #ifdef WIN32 */
#include <utime.h>
#endif /* #ifdef WIN32 */


#ifndef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#endif

#define TAG_TRACK_GAIN     "REPLAYGAIN_TRACK_GAIN"
#define TAG_TRACK_PEAK     "REPLAYGAIN_TRACK_PEAK"
#define TAG_ALBUM_GAIN     "REPLAYGAIN_ALBUM_GAIN"
#define TAG_ALBUM_PEAK     "REPLAYGAIN_ALBUM_PEAK"
#define TAG_TRACK_GAIN_OLD "RG_RADIO"
#define TAG_TRACK_PEAK_OLD "RG_PEAK"
#define TAG_ALBUM_GAIN_OLD "RG_AUDIOPHILE"

#define VALUE_BUFFER_SIZE 64
#define PEAK_FORMAT "%1.8f"
#define GAIN_FORMAT "%+2.2f dB"
#define PROGRESS_FORMAT " %3d%% - %s\r"
/* Size of PROGRESS_FORMAT when printed, excluding the filename */
#define PROGRESS_FORMAT_SIZE 8
#define MIN_FILENAME_SIZE 5
#define MIN_MIDDLE_TRUNCATE_SIZE 20
#define TEMP_NAME "vorbisgain.tmp"


/**
 * \brief See if a vorbis file contains ReplayGain tags.
 *
 * See if a vorbis file contain replay gain tags. A file is considered to
 * have the tags if all required tags (track gain and peak, optionally album
 * gain and peak) are present.
 *
 * \param filename   name of the file to check.
 * \param album_tags if true, check for the album tags (gain and peek) as
 *                   well.
 * \param any_tags   if true, the presence of any tag will be enough for a
 *                   file to contain tags. album_tags is ignored if any_tags
 *                   is true.
 * \param settings   global settings and data.
 * \return 1 if the file contains all the tags, 0 if the file didn't contain
 *         the tags and -1 if an error occured (in which case a message has
 *         been printed).
 */
int has_tags(const char* filename, int album_tags, int any_tags, 
    SETTINGS* settings)
{
    vcedit_state* state = NULL;
    vorbis_comment* vc;
    FILE* file = NULL;
    int found = 0;
    int expected;
    int result = -1;

    if (any_tags)
    {
        expected = 0;
    }
    else if (album_tags)
    {
        expected = 4;
    }
    else
    {
        expected = 2;
    }

    state = vcedit_new_state();

    if (state == NULL)
    {
        fprintf(stderr, _("Out of memory\n"));
        return result;
    }

    file = fopen(filename, "rb");

    if (file != NULL)
    {
        if (vcedit_open(state, file) >= 0)
        {
            int i;

            vc = vcedit_comments(state);

            for (i = 0; i < vc->comments; i++)
            {
                if(!tag_compare(vc->user_comments[i], TAG_TRACK_GAIN)
                    || !tag_compare(vc->user_comments[i], TAG_TRACK_PEAK))
                {
                    found++;
                }

                if((album_tags || any_tags) && 
                    (!tag_compare(vc->user_comments[i], TAG_ALBUM_GAIN) 
                        || !tag_compare(vc->user_comments[i], TAG_ALBUM_PEAK)))
                {
                    found++;
                }
            }

            result = (found >= expected) ? 1 : 0;
        }
        else
        {
            if (!settings->skip)
            {
                fprintf(stderr, _("Couldn't open file '%s' as vorbis: %s\n"),
                    filename, vcedit_error(state));
            }
        }

        fclose(file);
    }
    else
    {
        file_error(_("Couldn't open file '%s': "), filename);
    }

    vcedit_clear(state);
    return result;
}


/**
 * \brief Get old style tag values.
 *
 * Get the old style tag values from a file.
 *
 * \param filename   name of the file to check.
 * \param album_tag  if true, require file to contain album gain tag. If 
 *                   false, album gain is still returned if present.
 * \param track_gain track gain is stored here.
 * \param track_peak track peak is stored here.
 * \param album_gain album gain is stored here.
 * \param settings   global settings and data.
 * \return 1 if the file contains all the tags and their values coould be 
 *         parsed, 0 if the file didn't contain all the tags and -1 if an 
 *         error occured (in which case a message has been printed).
 */
int get_old_tags(const char* filename, int album_tag, float* track_peak,
     float* track_gain, float* album_gain, SETTINGS* settings)
{
    vcedit_state* state = NULL;
    vorbis_comment* vc;
    FILE* file = NULL;
    int found = 0;
    int expected = album_tag ? 3 : 2;
    int result = -1;

    state = vcedit_new_state();

    if (state == NULL)
    {
        fprintf(stderr, _("Out of memory\n"));
        return result;
    }

    file = fopen(filename, "rb");

    if (file != NULL)
    {
        if (vcedit_open(state, file) >= 0)
        {
            int i;

            vc = vcedit_comments(state);

            for (i = 0; i < vc->comments; i++)
            {
                if(!tag_compare(vc->user_comments[i], TAG_TRACK_GAIN_OLD))
                {
                    if (sscanf(&vc->user_comments[i][sizeof(TAG_TRACK_GAIN_OLD)],
                        "%f", track_gain) == 1)
                    {
                        found++;
                    }
                }

                if (!tag_compare(vc->user_comments[i], TAG_TRACK_PEAK_OLD))
                {
                    if (sscanf(&vc->user_comments[i][sizeof(TAG_TRACK_PEAK_OLD)],
                        "%f", track_peak) == 1)
                    {
                        found++;
                    }
                }

                if (!tag_compare(vc->user_comments[i], TAG_ALBUM_GAIN_OLD))
                {
                    if (sscanf(&vc->user_comments[i][sizeof(TAG_ALBUM_GAIN_OLD)],
                        "%f", album_gain) == 1)
                    {
                        found++;
                    }
                }
            }

            result = (found >= expected) ? 1 : 0;
        }
        else
        {
            if (!settings->skip)
            {
                fprintf(stderr, _("Couldn't open file '%s' as vorbis: %s\n"),
                    filename, vcedit_error(state));
            }
        }

        fclose(file);
    }
    else
    {
        file_error(_("Couldn't open file '%s': "), filename);
    }

    vcedit_clear(state);
    return result;
}

/**
 * \brief Show/update a simple progress bar for a file.
 *
 * Show/update a simple progress bar for a file. The name will be truncated 
 * to fit within the current console's with, if possible.
 *
 * \param vf         Vorbis file being processed.
 * \param filename   name of the file being processed.
 * \param position   position in the file, in percent (0-100).
 */
void show_progress(const char* filename, unsigned int position)
{
    unsigned int width;
    unsigned int height;
    char* short_name = NULL;
    const char* name = filename;

    if (!get_console_size(0, &width, &height))
    {
        unsigned int full_length = strlen(filename);

        if (full_length + PROGRESS_FORMAT_SIZE > width)
        {
            /* Need room for cursor as well. */
            int short_length = width - PROGRESS_FORMAT_SIZE - 1;
            
            /* Not useful to cut it too short. */
            short_length = MAX(short_length, MIN_FILENAME_SIZE + 3);
            short_name = malloc(short_length + 1);
            /* Need space for "..." */
            short_length -= 3;

            /* Put "..." in the middle if we have "enough" chars available. */
            if (short_length > MIN_MIDDLE_TRUNCATE_SIZE)
            {
                strncpy(short_name, filename, short_length / 2);
                short_name[short_length / 2] = '\0';
                strcat(short_name, "...");
                /* Round upwards here, so that all chars are used. */
                strcat(short_name, &filename[full_length 
                    - ((short_length + 1) / 2)]);
            }
            else
            {
                strcpy(short_name, "...");
                strcat(short_name, &filename[full_length - short_length]);
            }

            name = short_name;
        }
    }

    fprintf(stderr, _(PROGRESS_FORMAT), MIN(position, 100), name);

    if (NULL != short_name)
    {
        free(short_name);
    }
}

/**
 * Get the replaygain and peak value for a file.
 *
 * \param filename   name of the file to get the gain and peak for.
 * \param track_peak where to store track peak value.
 * \param track_gain where to store track gain value.
 * \param settings   global settings and data.
 * \return 0 if successful and -1 if an error occurred (in which case a
 *         message has been printed).
 */
int get_gain(const char* filename, float* track_peak, float* track_gain,
    SETTINGS* settings)
{
    OggVorbis_File vf;
    FILE* file = NULL;
    vorbis_info* vi = NULL;
    ogg_int64_t file_length = -1;
    float peak = 0.f;
    float scale;
    float new_peak;
    long rate;
    int previous_percentage = -1;
    time_t previous_time = 0;
    int previous_section = 0;
    int current_section;
    int channels;
    int result;

    file = fopen(filename, "rb");

    if (file == NULL)
    {
        fprintf(stderr, _("Couldn't open file '%s'\n"), filename);
        return -1;
    }

    result = ov_open(file, &vf, NULL, 0);

    if(result < 0)
    {
        if (!settings->skip)
        {
	        vorbis_error(result, _("Couldn't process '%s': "), filename);
        }

        return -1;
    }

    vi = ov_info(&vf, -1);

    if ((vi->channels != 1) && (vi->channels != 2))
    {
        fprintf(stderr, _("Unsupported number of channels (%d) in '%s'.\n"), 
            vi->channels, filename);
        ov_clear(&vf);
        return -1;
    }

    /* Only initialize gain analysis once per album, when in album mode */
    if (settings->first_file || !settings->album)
    {
        if (InitGainAnalysis(vi->rate) != INIT_GAIN_ANALYSIS_OK)
        {
            fprintf(stderr, _("Couldn't initialize gain analysis"
                " (nonstandard samplerate?) for '%s'\n"), filename);
            ov_clear(&vf);
            return -1;
        }
    }

    channels = vi->channels;
    rate = vi->rate;

    if (settings->first_file)
    {
        if (!settings->quiet)
        {
            fprintf(stdout, _("Analyzing files...\n\n"));
            fprintf(stdout, _("   Gain   |  Peak  | Scale | New Peak | Track\n"));
            fprintf(stdout, _("----------+--------+-------+----------+------\n"));
        }

        settings->first_file = 0;
    }

    if (!settings->quiet)
    {
        file_length = ov_pcm_total(&vf, -1);

        if (settings->show_progress && (file_length > 0) 
	    && (ov_pcm_tell(&vf) >= 0))
        {
            show_progress(filename, 0);
        }
    }

    while (1)
    {
        float** pcm;
        float* left_pcm;
        float* right_pcm = NULL;
        long ret;

#ifdef VORBISFILE_OV_READ_FLOAT_3_ARGS
        ret = ov_read_float(&vf, &pcm, &current_section);
#else
        ret = ov_read_float(&vf, &pcm, 1024, &current_section);
#endif

        left_pcm = pcm[0];

        if (vi->channels == 2)
        {
            right_pcm = pcm[1];
        }

        if (ret == 0)
        {
            break;
        }
        else if (ret < 0)
        {
            /* Error in the stream. Not a problem, just reporting it in case
             * we (the app) cares. In this case, we don't
             */
        }
        else
        {
            long i;

            /* We can't handle changes in sample rate or number of channels.
             * Channels would probably be OK (at least 1 to 2 and vice versa),
             * but... :)
             */
            if (current_section != previous_section)
            {
                previous_section = current_section;
                vi = ov_info(&vf, -1);

                if ((channels != vi->channels) || (rate != vi->rate))
                {
                    fprintf(stderr, _("Can't process chained file '%s'\n"
                        "with changing stream properties (from %ld Hz, "
                        "%d channel(s)\nto %ld Hz, %d channel(s).\n"),
                        filename, rate, channels, vi->rate, vi->channels);
                    ov_clear(&vf);
                    return -1;
                }
            }

            for (i = 0; i < ret; i++)
            {
                left_pcm[i] *= 32767.;

                if (fabs(left_pcm[i]) > peak)
                {
                    peak = (float) fabs(left_pcm[i]);
                }

                if (vi->channels == 2)
                {
                    right_pcm[i] *= 32767.;

                    if (fabs(right_pcm[i]) > peak)
                    {
                        peak = (float) fabs(right_pcm[i]);
                    }
                }
            }

            if (AnalyzeSamples(left_pcm, right_pcm, ret, vi->channels)
                != GAIN_ANALYSIS_OK)
            {
                fprintf(stderr, _("Couldn't analyze samples in file '%s'\n"), 
                    filename);
                ov_clear(&vf);
                return -1;
            }
        }

        if (file_length > 0)
        {
            /* ov_time_tell is slow, so we use ov_pcm_tell instead */
            ogg_int64_t file_pos = ov_pcm_tell(&vf);

            if (file_pos > 0)
            {
                int percentage = (int) floor((((double) file_pos / file_length) * 100) + 0.5);
                time_t curr_time;

                time(&curr_time);

                if ((100 == percentage) || ((percentage != previous_percentage) 
                    && (difftime(curr_time, previous_time) >= 1)))
                {
		  if (settings->show_progress) 
		  {
                    show_progress(filename, percentage);
		  }
		  previous_percentage = percentage;
		  previous_time = curr_time;
                }
            }
        }
    }

    *track_gain = GetTitleGain();
    scale = (float) pow(10., *track_gain / 20.);
    *track_peak = (float) (peak / 32767.);
    new_peak = peak * scale;

    if (!settings->quiet)
    {
        fprintf(stdout, _("%+6.2f dB | %6.0f | %5.2f | %8.0f | %s\n"),
            *track_gain, peak, scale, new_peak, filename);
    }

    ov_clear(&vf);
    return 0;
}


/**
 * \brief Write the replay gain tags to a file.
 *
 * Write the replay gain tags to a file, removing any old style tags. Any
 * other tags remain unchanged. If track_gain or album_gain contains NO_GAIN,
 * no corresponding tag is written. If track_peak or album_peak contains
 * NO_PEAK, no corresponding tag is written. However, if NO_GAIN or NO_PEAK is
 * specified, any already present tag of the corresponding type will remain.
 *
 * \param filename    name of the file to write the tags to.
 * \param track_peak  track peak value to write, or NO_PEAK.
 * \param track_gain  track replay gain value to write, or NO_GAIN.
 * \param album_peak  album peak value to write, or NO_PEAK.
 * \param album_gain  album gain value to write, or NO_GAIN.
 * \param verbose     print processing messages.
 * \param remove_tags if true, remove all replay gain tags from the file 
 *                    (both old and new style). Any track or album values are
 *                    ignored.
 * \return 0 if successful and -1 if an error occurred (in which case a
 *         message has been printed).
 */
int write_gains(const char *filename, float track_peak, float track_gain,
    float album_peak, float album_gain, int verbose, int remove_tags)
{
    struct stat stat_buf;
    struct utimbuf utime_buf;
    vcedit_state* state = NULL;
    vorbis_comment* vc;
    FILE* infile = NULL;
    FILE* outfile = NULL;
    char** new_tags = NULL;
    char* temp_name = NULL;
    char buffer[VALUE_BUFFER_SIZE];
    int new_count = 0;
    int result = -1;
    int delete_temp = 0;
    int i;

    infile = fopen(filename, "rb");

    if (infile == NULL)
    {
        file_error(_("Couldn't open '%s' for input: "), filename);
        goto exit;
    }

    state = vcedit_new_state();

    if (vcedit_open(state, infile) < 0)
    {
        fprintf(stderr, _("Couldn't open file '%s' as vorbis: %s\n"),
            filename, vcedit_error(state));
        goto exit;
    }

    vc = vcedit_comments(state);
    new_tags = (char**) calloc(vc->comments, sizeof(char*));

    for (i = 0; i < vc->comments; i++)
    {
        /* Copy all tags, except those we wish to change. 
         * Alternatively, remove all ReplayGain tags.
         */
        int copy_tag = 1;

        /* Remove any old style tags */

        if (!tag_compare(vc->user_comments[i], TAG_TRACK_GAIN_OLD)
            || !tag_compare(vc->user_comments[i], TAG_TRACK_PEAK_OLD)
            || !tag_compare(vc->user_comments[i], TAG_ALBUM_GAIN_OLD))
        {
            copy_tag = 0;
        }

        /* Check for tags */

        if (!tag_compare(vc->user_comments[i], TAG_TRACK_GAIN)
            && (remove_tags || (track_gain > NO_GAIN)))
        {
            copy_tag = 0;
        }

        if (!tag_compare(vc->user_comments[i], TAG_TRACK_PEAK)
            && (remove_tags || (track_peak > NO_PEAK)))
        {
            copy_tag = 0;
        }

        if (!tag_compare(vc->user_comments[i], TAG_ALBUM_GAIN)
            && (remove_tags || (album_gain > NO_GAIN)))
        {
            copy_tag = 0;
        }

        if (!tag_compare(vc->user_comments[i], TAG_ALBUM_PEAK)
            && (remove_tags || (album_peak > NO_PEAK)))
        {
            copy_tag = 0;
        }

        if (copy_tag)
        {
            new_tags[new_count] = strdup(vc->user_comments[i]);

            if (new_tags[new_count++] == NULL)
            {
                fprintf(stderr, _("Out of memory\n"));
                goto exit;
            }
        }
    }

    vorbis_comment_clear(vc);
    vorbis_comment_init(vc);

    /* Add the old tags to the new file */
    for (i = 0; i < new_count; i++)
    {
        vorbis_comment_add(vc, new_tags[i]);
    }

    /* Add new tags - unless ReplayGain tags are to be removed */
    if (!remove_tags)
    {
        if (track_peak > NO_PEAK)
        {
            sprintf(buffer, PEAK_FORMAT, track_peak);
            vorbis_comment_add_tag(vc, TAG_TRACK_PEAK, buffer);
        }

        if (track_gain > NO_GAIN)
        {
            sprintf(buffer, GAIN_FORMAT, track_gain);
            vorbis_comment_add_tag(vc, TAG_TRACK_GAIN, buffer);
        }

        if (album_peak > NO_PEAK)
        {
            sprintf(buffer, PEAK_FORMAT, album_peak);
            vorbis_comment_add_tag(vc, TAG_ALBUM_PEAK, buffer);
        }

        if (album_gain > NO_GAIN)
        {
            sprintf(buffer, GAIN_FORMAT, album_gain);
            vorbis_comment_add_tag(vc, TAG_ALBUM_GAIN, buffer);
        }
    }

    /* Make sure temp is in same folder as file. And yes, the malloc is larger
     * than necessary (and not always needed). Lets keep it simple though (at
     * the expense of a few bytes)...
     */
    temp_name = malloc(strlen(filename) + sizeof(TEMP_NAME));

    if (temp_name == NULL)
    {
        fprintf(stderr, _("Out of memory\n"));
        goto exit;
    }

    strcpy(temp_name, filename);
    strcpy((char *) last_path(temp_name), TEMP_NAME);

    outfile = fopen(temp_name, "wb");

    if (outfile == NULL)
    {
        file_error(_("Couldn't open '%s' for output: "), temp_name);
        goto exit;
    }

    if (verbose)
    {
        fprintf(stderr, remove_tags
            ? _("Removing tags from '%s'\n")
            : _("Writing tags to '%s'\n") , filename);
    }

    if (vcedit_write(state, outfile) < 0)
    {
        /* Not sure if file_error is useful here. vcedit_error() (currently)
         * won't give anything useful though.
         */
        file_error(_("Couldn't write replay gain tags for '%s': "), filename);
        delete_temp = 1;
        goto exit;
    }

    vcedit_clear(state);
    state = NULL;
    fclose(infile);
    infile = NULL;

    /* Only bother to check for close failure on the output file */
    i = fclose(outfile);

    if (i != 0)
    {
        file_error(_("Couldn't write replay gain tags for '%s': "), filename);
        outfile = NULL;
        delete_temp = 1;
        goto exit;
    }

    outfile = NULL;

    if (stat(filename, &stat_buf) != 0)
    {
        file_error(_("Couldn't get information about old file '%s': "), filename);
        goto exit;
    }

    if (remove(filename) != 0)
    {
        file_error(_("Couldn't delete old file '%s': "), filename);
        goto exit;
    }

    if (rename(temp_name, filename) != 0)
    {
        file_error(_("Couldn't rename '%s' to '%s': "), temp_name, filename);
        goto exit;
    }

    /* TRY to copy mode and modification time... */

    if (chmod(filename, stat_buf.st_mode) != 0)
    {
        file_error(_("Note: Couldn't set mode for file '%s': "), filename);
    }

    utime_buf.actime = stat_buf.st_atime;
    utime_buf.modtime = stat_buf.st_mtime;

    if (utime(filename, &utime_buf) != 0)
    {
        file_error(_("Note: Couldn't set time for file '%s': "), filename);
    }

    result = 0;

exit:
    if (new_tags != NULL)
    {
        for (i = 0; i < new_count; ++i)
        {
            free(new_tags[i]);
        }

        free(new_tags);
    }

    if (state != NULL)
    {
        vcedit_clear(state);
    }

    if (infile != NULL)
    {
        fclose(infile);
    }

    if (infile != NULL)
    {
        fclose(infile);
    }

    if (delete_temp)
    {
        if (remove(TEMP_NAME) != 0)
        {
            file_error(_("Note: Couldn't remove temporary file '%s': "), 
                TEMP_NAME);
        }
    }

    free(temp_name);

    return result;
}
