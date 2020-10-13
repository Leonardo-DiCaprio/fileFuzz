#ifndef VG_MISC_H
#define VG_MISC_H

void file_error(const char* message, ...);
void vorbis_error(int vorbis_error, const char* message, ...);
char* last_path(const char* path);
int tag_compare(const char* s1, const char* s2);

#ifndef DISABLE_TRUNCATE
int get_console_size(int get_err, unsigned int* columns, unsigned int* rows);
#endif

#endif /* VG_MISC_H */

