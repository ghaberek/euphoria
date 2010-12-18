#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef MAX_PATH
#define MAX_PATH 1024
#endif

#define BUF_SIZE 256
#define SHORT_SIZE 12

int has_be_ver()
{
    FILE *ver_fh;

    ver_fh = fopen("be_ver.h", "r");
    if (ver_fh == NULL)
        return 0;
    fclose(ver_fh);
    return 1;
}

int get_version_info(char *cache_filename,
					 char *version, int version_size,
                     char *date, int date_size)
{
    char buf[256];
    char *tmp;
    FILE *ver_fh;

    ver_fh = fopen(cache_filename, "r");
    if (ver_fh == NULL)
    {
        return 0;
    }

    fgets(buf, BUF_SIZE, ver_fh);

    /* trim the newline */
    if (strlen(buf) < version_size)
        buf[strlen(buf)-1] = 0;
    strncpy(version, buf, version_size);

    fgets(buf, BUF_SIZE, ver_fh);
    strncpy(date, buf, date_size);

    fclose(ver_fh);

    return 1;
}

void put_version(const char *output_filename, const char *version,
                 const char *date)
{
    FILE *ver_fh;
    char version_short[SHORT_SIZE + 1];

	strncpy(version_short, version, SHORT_SIZE);
	version_short[SHORT_SIZE] = 0;

    ver_fh = fopen(output_filename, "w");
    if (ver_fh == NULL)
    {
        fprintf(stderr, "Could not open for writing %s\n", output_filename);
        exit(1);
    }

    fprintf(ver_fh, "/* auto-generated by mkver, do not edit */\n");
    fprintf(ver_fh, "#ifndef SCM_VER_FULL\n");
    fprintf(ver_fh, "#define SCM_VER_FULL \"%s\"\n", version);
    fprintf(ver_fh, "#define SCM_VER \"%s\"\n", version_short);
    fprintf(ver_fh, "#define SCM_DATE \"%s\"\n", date);
    fprintf(ver_fh, "#endif\n");
    fclose(ver_fh);

}

int main(int argc, char **argv)
{
    FILE *ver_fh;
    char tmp[MAX_PATH];
    char *hg_executable, *cache_filename, *output_filename;
    char old_ver[BUF_SIZE], old_date[BUF_SIZE];
    char new_ver[BUF_SIZE], new_date[BUF_SIZE];
    int had_old_info = 0;

    if (argc < 4)
    {
        fprintf(stderr, "Usage: %s hg-executable cache-file output-filename\n",
                argv[0]);
        exit(1);
    }

	hg_executable = argv[1];
	cache_filename = argv[2];
    output_filename = argv[3];

    /*
     * We get our old information to compare to the new information.
     * If there is no difference, then we do not write a new version
     * file which prevents make dependencies from building be_machine.c
     * over and over.
     */

    had_old_info = get_version_info(cache_filename, old_ver, BUF_SIZE, old_date, BUF_SIZE);

#if defined(__WATCOMC__) || defined(__MINGW32__)
    snprintf(tmp, MAX_PATH,
			 "\"%s\" parents --template {node}\\n{date^|shortdate} > %s",
			 hg_executable, cache_filename);
#else
	snprintf(tmp, MAX_PATH,
			 "%s parents --template '{node}\n{date|shortdate}' > %s",
			 hg_executable, cache_filename);
#endif

    if (system(tmp) != 0)
    {
        /*
         * If we do not have a be_ver.h, write a junk one. Otherwise,
         * leave the existing one alone... i.e. from a source-tarball?
         */
        if (has_be_ver() == 0)
            put_version(output_filename, "unknown", "unknown");

        exit(0);
    }

    if (get_version_info(cache_filename, new_ver, BUF_SIZE, new_date, BUF_SIZE) == 0)
    {
	  fprintf(stderr, "Could not open cache file %s\n  result of hg parents\n",
			  cache_filename);
	  exit(1);
    }

    if (has_be_ver() == 0 ||
        had_old_info == 0 ||
        strncmp(new_ver, old_ver, BUF_SIZE) != 0)
    {
        put_version(output_filename, new_ver, new_date);
    }

    return 0;
}
