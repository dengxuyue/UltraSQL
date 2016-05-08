#include "profile.h"
#include <string.h>
#include <stdio.h>

static char setting_list[SETTING_ENTRY_LIMIT][PROFILE_LINE_LENGTH];
static int  setting_count = 0;
static int  setting_index = 0;

int append_setting_list (char* line)
{
    if (setting_count >= SETTING_ENTRY_LIMIT)
        return 1;

    int i= 0, len= strlen(line);
    while (line[i] != '\0' && (line[i] == ' ' || line [i] == '\t'))
        i++;

    if (line[i] != '.')
        return -1;

    if (len - i < 5) /*'.set q'*/
        return -1;

    if (strncasecmp(line + i + 1, "set", 3))
        return -1;

    strncpy(setting_list[setting_count], line + i, len - i);
    setting_list[setting_count][len - i] = '\0';
    setting_count++;

    return 0;
}


char* get_next_setting_item ()
{
    char* ptr = 0;
    if(setting_count > 0 && setting_index < setting_count)
        ptr = setting_list[setting_index++];

    return ptr;
}

