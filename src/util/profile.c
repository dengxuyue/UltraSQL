#include "profile.h"
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * The enum data and 'internal_section_type' should be updated consistently
 */
enum {
    password$ = 0,
    setting$,

    /* The last two items! */
    type_count$,
    unknown$,
};

static char* internal_section_heading[] = 
{
    "password",
    "setting",
};

int internal_section_type = unknown$;


/*
 * 1) find the section type  -- return value
 * 2) find out whether the current line is empty -- argument: null_line
 */
static int find_section_type (char* line, int* null_line) 
{
    int i, len = strlen(line);
    int start, end, max;
    start = end = 0;
    *null_line = 1;
    for (i = 0; i < len; i++) {
        if (!(line[i] == ' ' || line[i] == '\t' || line[i] == '\n' || line[i] == '\r'))
            *null_line = 0;

        if (line[i] == '[')
            start = i + 1;
        else if (line[i] == ']') 
            end = i - 1;
    }

    if (start < end) {
        for(i = 0; i < type_count$; i++) {
            max = strlen(internal_section_heading[i]);
            if (max > end - start + 1) 
                max = end - start + 1;
            if (!strncasecmp(line + start, internal_section_heading[i], max)) {
                return i;
            }
        }
        
    }

    return -1;
}


int init_profile () 
{
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;
    char usql_profile[128];
    sprintf(usql_profile, "%s/%s", pw->pw_dir, USQL_PROFILE);

    FILE * pf;
    if(!(pf = fopen(usql_profile, "r")))
        return 1;

    int line_len, max_len;
    char* line;
    int line_no = 0;
    int new_section_type, null_line;
    while (1) {
        line = 0;
        max_len = 0;
        line_len = getline(&line, &max_len, pf);
        if (line_len == -1)
            break;
        if (line_len > PROFILE_LINE_LENGTH) 
            goto errout;

        line_no++;

        /* null_line is updated to indicate whether current line is empty */
        if ((new_section_type = find_section_type(line, &null_line)) != -1) {
            internal_section_type = new_section_type;
            free(line);
            continue;
        }

        if (internal_section_type == password$) {
            if (null_line == 0 && add_dbs_passwd(line) == -1)
                goto errout;
        }
        else if (internal_section_type == setting$) { 
            if (null_line == 0 && append_setting_list(line) == -1)
                goto errout;
        }

        free(line);
    }
   
    fclose(pf);
    return 0;

errout:
    /* getline clears buffer usql_profile */
    sprintf(usql_profile, "%s/%s", pw->pw_dir, USQL_PROFILE);
    if(!line)
        free(line);
    fprintf(stderr, "Syntax error in Line %d of %s.\n", line_no, usql_profile);
    fclose(pf);
    return -1;
}


void fini_profile ()
{

}

