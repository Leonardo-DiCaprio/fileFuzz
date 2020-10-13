/*
 * function: ReplayGain support for Vorbis (http://www.replaygain.org)
 *
 * Reads in a Vorbis file, figures out the peak and ReplayGain levels and
 * writes the ReplayGain tags to the file
 *
 * This program is distributed under the GNU General Public License, version 
 * 2.1. A copy of this license is included with this source.
 *
 * Copyright (C) 2002-2003 Gian-Carlo Pascutto and Magnus Holmgren
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <ctype.h>
#include "getopt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gain_analysis.h"
#include "i18n.h"
#include "misc.h"
#include "vorbis.h"
#include "vorbisgain.h"

#ifdef WIN32
#include <windows.h>
#endif

#ifdef ENABLE_RECURSIVE
#include "recurse.h"
#endif


/* Some version numbers */
#define VG_VERSION     "0.36"
#define VORBIS_VERSION "1.0"


/**
 * \brief Allocate a FILE_LIST node.
 *
 * Allocate a FILE_LIST node. filename is set to file while track_peak and
 * track_gain are set to NO_PEAK and NO_GAIN respectively. The rest of the
 * fields are set to zero.
 *
 * \param file  name of file to get a node for.
 * \return  the allocated node or NULL (in which case a message has been
 *          printed).
 */
static FILE_LIST* alloc_node(const char* file)
{
    FILE_LIST* node;

    node = calloc(1, sizeof(*node));

    if (node != NULL)
    {
        node->filename = strdup(file);

        if (node->filename != NULL)
        {
            node->track_peak = NO_PEAK;
            node->track_gain = NO_GAIN;
            return node;
        }

        free(node);
    }

    fprintf(stderr, _("Out of memory\n"));
    return NULL;
}


/**
 * \brief Allocate a new FILE_LIST node and add it to the end of a list.
 *
 * Allocate a new FILE_LIST node and add it to the end of a list.
 *
 * \param list  pointer to the list (first node) pointer. If the list
 *              pointer contains NULL, it is set to the new node. Otherwise 
 *              the new node is added the last node in the list.
 * \param file  name of the file to get a node for.
 * \return  0 if successful and -1 if the node couldn't be allocated (in
 *          which case a message has been printed).
 */
int add_to_list(FILE_LIST** list, const char* file)
{
    if (*list == NULL)
    {
        *list = alloc_node(file);

        if (*list == NULL)
        {
            return -1;
        }
    }
    else
    {
        FILE_LIST* node = *list;

        while (node->next_file != NULL)
        {
            node = node->next_file;
        }

        node->next_file = alloc_node(file);

        if (node->next_file == NULL)
        {
            return -1;
        }
    }

    return 0;
}


/**
 * Free all nodes in a FILE_LIST list.
 *
 * \param list  pointer to first node in list. Can be NULL, in which case 
 *              nothing is done.
 */
void free_list(FILE_LIST* list)
{
    FILE_LIST* next;

    while (list != NULL)
    {
        next = list->next_file;
        free((void *) list->filename);
        free(list);
        list = next;
    }
}


/**
 * \brief Convert tags from old to new style.
 *
 * Convert tags from old to new style. Checks that the album gain is the same
 * for all files, when in album mode.
 *
 * \param files     list of files to process.
 * \param settings  global settings and data.
 * \return  0 if successful and -1 if an error occured (in which case a
 *          message has been printed).
 */
int convert_files(FILE_LIST* file_list, SETTINGS* settings)
{
    FILE_LIST* file;
    float album_gain = NO_GAIN;
    float album_peak = NO_PEAK;
    int convert = 0;

    if (file_list == NULL)
    {
        return 0;
    }


    for (file = file_list; file; file = file->next_file)
    {
        int tags = get_old_tags(file->filename, settings->album, 
            &file->track_peak, &file->track_gain, &file->album_gain, settings);

        if (tags < 0)
        {
            if (settings->skip)
            {
                file->skip = 1;
                continue;
            }
            else
            {
                return -1;
            }
        }

        file->skip = !tags;
        convert |= tags;

        if (settings->album)
        {
            if (!tags)
            {
                fprintf(stderr, _("Tags required for album mode conversion not "
                    "found in '%s'\n"), file->filename);
                return -1;
            }

            if ((album_gain > NO_GAIN) && (file->album_gain != album_gain))
            {
                fprintf(stderr, _("Album gain in '%s' differs from previous "
                    "files (%.2f vs %.2f)\n"), file->filename, 
                    file->album_gain, album_gain);
                return -1;
            }

            album_gain = file->album_gain;

            if (file->track_peak > album_peak)
            {
                album_peak = file->track_peak;
            }
        }
    }

    if (!convert)
    {
        fprintf(stderr, _("No old style ReplayGain tags found; no files "
            "processed\n"));
        return 0;
    }

    for (file = file_list; file; file = file->next_file)
    {
        if (file->skip)
        {
            continue;
        }

        if (settings->display_only)
        {
            if (!settings->quiet)
            {
                fprintf(stderr, _("Found old style ReplayGain tags in '%s'\n"),
                    file->filename);
            }
        }
        else
        {
            if (write_gains(file->filename, file->track_peak, 
                file->track_gain, album_peak, file->album_gain, 
                !settings->quiet, 0) < 0)
            {
                return -1;
            }
        }
    }

    return 0;
}


/**
 * Remove VorbisGain tags from files.
 *
 * \param files     list of files to process.
 * \param settings  global settings and data.
 * \return  0 if successful and -1 if an error occured (in which case a
 *          message has been printed).
 */
int clean_files(FILE_LIST* file_list, SETTINGS* settings)
{
    FILE_LIST* file;

    /* Remove ReplayGain tags */
    for (file = file_list; file; file = file->next_file)
    {
        int tags = has_tags(file->filename, 0, 1, settings);

        if (tags < 0)
        {
            if (settings->skip)
            {
                tags = 0;
            }
            else
            {
                return -1;
            }
        }

        if (tags)
        {
            if (settings->display_only)
            {
                if (!settings->quiet)
                {
                    fprintf(stderr, _("Found ReplayGain tags in '%s'\n"), 
                        file->filename);
                }
            }
            else
            {
                if (write_gains(file->filename, NO_PEAK, NO_GAIN, 
                    NO_PEAK, NO_GAIN, !settings->quiet, 1) < 0)
                {
                    return -1;
                }
            }
        }
        else
        {
            if (!settings->quiet)
            {
                fprintf(stderr, _("No ReplayGain tags to remove in '%s'\n"),
                    file->filename);
            }
        }
    }

    return 0;
}


/**
 * \brief Processs the file in file_list.
 *
 * Process the files in file_list. If settings->album is set, the files are
 * considered to belong to one album.
 *
 * \param file_list  list of files to process.
 * \param settings   settings and global variables.
 * \return  0 if successful and -1 if an error occured (in which case a
 *          message has been printed).
 */
int process_files(FILE_LIST* file_list, SETTINGS* settings)
{
    FILE_LIST* file;
    float album_peak = NO_PEAK;
    int analyze = 1;

    if (file_list == NULL)
    {
        return 0;
    }

    if (settings->convert)
    {
        return convert_files(file_list, settings);
    }

    if (settings->clean)
    {
        return clean_files(file_list, settings);
    }

    settings->first_file = 1;

    if (settings->fast)
    {
        analyze = 0;

        /* Figure out which files need to be processed */
        for (file = file_list; file; file = file->next_file)
        {
            int tags = has_tags(file->filename, settings->album, 0, settings);

            if (tags < 0)
            {
                /* Skip non-vorbis files in recursive mode */
                if (settings->skip)
                {
                    file->skip = 1;
                }
                else
                {
                    return tags;
                }
            }

            analyze |= !tags;

            if (settings->album && analyze)
            {
                /* In album mode, all tags must be present in all files (this
                 * check isn't necessary, but avoids unneeded tag searches)
                 */
                break;
            }

            if (tags && !settings->album)
            {
                file->skip = 1;
            }
        }

        if (!analyze && !settings->album_gain_set)
        {
            /* All files fully tagged and album gain doesn't need to be changed */
            if (!settings->quiet)
            {
                fprintf(stderr, _("Tags present; no files processed\n"));
            }

            return 0;
        }
    }

    if (analyze)
    {
        /* Analyze the files, write track gain if not in album mode */
        for (file = file_list; file; file = file->next_file)
        {
            if (file->skip)
            {
                continue;
            }

            if (get_gain(file->filename, &file->track_peak, &file->track_gain,
                settings) < 0)
            {
                /* Skip non-vorbis and erraneous files. */
                if (settings->skip)
                {
                    file->skip = 1;
                    continue;
                }
                else
                {
                    return -1;
                }
            }

            if (settings->album && (file->track_peak > album_peak))
            {
                album_peak = file->track_peak;
            }

            if (!settings->album)
            {
                if (!settings->display_only && write_gains(file->filename,
                    file->track_peak, file->track_gain, NO_PEAK, NO_GAIN, 0, 0) < 0)
                {
                    return -1;
                }
            }
        }
    }

    if (settings->album)
    {
        /* Write track and album gains */
        char* gain_message;
        float album_gain;

        if (settings->album_gain_set)
        {
            album_gain = settings->album_gain;
            gain_message = "\nSetting Album Gain to: %+2.2f dB\n";
        }
        else
        {
            album_gain = GetAlbumGain();
            gain_message = "\nRecommended Album Gain: %+2.2f dB\n";
        }

        if (!settings->quiet)
        {
            fprintf(stderr, _(gain_message), album_gain);
        }

        if (!settings->display_only)
        {
            for (file = file_list; file; file = file->next_file)
            {
                if (file->skip)
                {
                    continue;
                }

                if (write_gains(file->filename, file->track_peak,
                    file->track_gain, album_peak, album_gain,
                    !settings->quiet, 0) < 0)
                {
                    return -1;
                }
            }
        }
    }

    return 0;
}


/**
 * Print out a list of options and the command line syntax.
 */
static void print_help(void)
{
    fprintf(stderr, _("VorbisGain v" VG_VERSION " (libvorbis " VORBIS_VERSION ")\n"));
    fprintf(stderr, _("Copyright (c) 2002-2005 Gian-Carlo Pascutto <gcp@sjeng.org>\n"));
    fprintf(stderr, _("                        and Magnus Holmgren <lear@algonet.se>\n\n"));
    fprintf(stderr, _("Usage: vorbisgain [options] input.ogg [...]\n\n"));
    fprintf(stderr, _("OPTIONS:\n"));
    fprintf(stderr, _("  -a, --album          Run in Album (Audiophile) mode.\n"));
    fprintf(stderr, _("  -g, --album-gain=n   Set Album gain to the specified value. Implies -a\n"));
    fprintf(stderr, _("  -c, --clean          Remove any VorbisGain tags. No gains are calculated\n"));
    fprintf(stderr, _("  -C, --convert        Convert VorbisGain tags from old to new style\n"));
    fprintf(stderr, _("  -d, --display-only   Display results only. No files are modified\n"));
    fprintf(stderr, _("  -f, --fast           Don't recalculate tagged files\n"));
    fprintf(stderr, _("  -h, --help           Print this help text\n"));
    fprintf(stderr, _("  -n, --no-progress    Don't show progress, just print results\n"));
    fprintf(stderr, _("  -q, --quiet          Don't print any output (except errors)\n"));
#ifdef ENABLE_RECURSIVE
    fprintf(stderr, _("  -r, --recursive      Search for files recursivly, each folder as an album\n"));
#endif
    fprintf(stderr, _("  -s, --skip           Skip non-Vorbis or faulty files\n"));
    fprintf(stderr, _("  -v, --version        Display version number and exit\n\n"));
    fprintf(stderr, _("INPUT FILES:\n"));
    fprintf(stderr, _("  ReplayGain input files must be Ogg Vorbis I files with\n"));
    fprintf(stderr, _("  1 or 2 channels and a sample rate of 48 kHz, 44.1 kHz,\n"));
    fprintf(stderr, _("  32 kHz, 24 kHz, 22050 Hz, 16 kHz, 12 kHz, 11025 Hz or 8 kHz.\n"));
    fprintf(stderr, _("  Wildcards (?, *) can be used in the filename.\n\n"));

    return;
}


const static struct option long_options[] =
{
    {"album",        0, NULL, 'a'},
    {"album-gain",   1, NULL, 'g'},
    {"clean",        0, NULL, 'c'},
    {"convert",      0, NULL, 'C'},
    {"display-only", 0, NULL, 'd'},
    {"fast",         0, NULL, 'f'},
    {"help",         0, NULL, 'h'},
    {"no-progress",  0, NULL, 'n'},
    {"quiet",        0, NULL, 'q'},
#ifdef ENABLE_RECURSIVE
    {"recursive",    0, NULL, 'r'},
#endif
    {"skip",         0, NULL, 's'},
    {"version",      0, NULL, 'v'},
    {NULL,           0, NULL, 0}
};


#ifdef ENABLE_RECURSIVE
#define ARG_STRING "acCdfg:hnqrst:v"
#else
#define ARG_STRING "acCdfg:hnqst:v"
#endif


int main(int argc, char** argv)
{
    SETTINGS settings;
    int option_index = 1;
    int ret;
    int i;

    memset(&settings, 0, sizeof(settings));
    settings.first_file = 1;
    settings.album_gain = NO_GAIN;
    settings.show_progress = 1;

#ifdef WIN32
    /* Is this good enough? Or do we need to consider multi-byte codepages as 
     * well? 
     */
    SetConsoleOutputCP(GetACP());
#endif

    while ((ret = getopt_long(argc, argv, ARG_STRING, long_options, &option_index)) != -1)
    {
        switch(ret)
        {
        case 0:
            fprintf(stderr, _("Internal error parsing command line options\n"));
            return EXIT_FAILURE;
            break;

        case 'a':
            settings.album = 1;
            break;

        case 'g':
            settings.album = 1;

            if (sscanf(optarg, "%f", &settings.album_gain) != 1)
            {
                fprintf(stderr, _("Album gain \"%s\" not recognised, "
                    "ignoring\n"), optarg);
                settings.album_gain = NO_GAIN;
                break;
            }

            /* Check the value for sanity. */
            if ((settings.album_gain < -51.0) || (settings.album_gain > 51.0))
            {
                fprintf(stderr, _("Album gain \"%s\" is out of range (-51 to "
                    "51 dB), ignoring\n"), optarg);
                settings.album_gain = NO_GAIN;
            }
            else
            {
                settings.album_gain_set = 1;

                if ((settings.album_gain < -23.0) || (settings.album_gain > 17.0))
                {
                    fprintf(stderr, _("Note: An album gain of \"%s\" is unlikely (limit the album gain\n"), 
                        optarg);
                    fprintf(stderr, _("      to be within the range -23 to 17 dB to avoid this warning)\n"));
                }
            }

            break;

        case 'c':
            settings.clean = 1;
            break;

        case 'C':
            settings.convert = 1;
            break;

        case 'd':
            settings.display_only = 1;
            break;

        case 'f':
            settings.fast = 1;
            break;

	case 'n':
	  settings.show_progress = 0;
	  break;

        case 'q':
            settings.quiet = 1;
            break;

#ifdef ENABLE_RECURSIVE
        case 'r':
            settings.recursive = 1;
            break;
#endif

        case 's':
            settings.skip = 1;
            break;

        case 'v':
	        fprintf(stderr, "VorbisGain v" VG_VERSION " (libvorbis " VORBIS_VERSION ")\n");
            return EXIT_SUCCESS;

        default:
            print_help();
            return EXIT_SUCCESS;
        }
    }

    if (optind >= argc)
    {
        fprintf(stderr, _("No files specified.\n"));
        print_help();
        return EXIT_SUCCESS;
    }

    for (i = optind; i < argc; ++i)
    {
#ifdef ENABLE_RECURSIVE
        if (process_argument(argv[i], &settings) < 0)
        {
            free_list(settings.file_list);
            return EXIT_FAILURE;
        }
#else
        if (add_to_list(&settings.file_list, argv[i]) < 0)
        {
            return EXIT_FAILURE;
        }
#endif
    }

    /* Process files (perhaps remaining) in list */
    ret = process_files(settings.file_list, &settings);
    free_list(settings.file_list);
    settings.file_list = NULL;

    return (ret < 0) ? EXIT_FAILURE : EXIT_SUCCESS;
}
