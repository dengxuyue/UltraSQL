#include "ext_lib.h"
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

static char usql_history[HISTORY_PATH_LENGTH];
extern int history_length;

int init_history()
{
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;
    sprintf(usql_history, "%s/%s", pw->pw_dir, USQL_HISTORY);
    
    using_history();
    read_history(usql_history);
	history_set_pos(0);

    return 0;
}

void fini_history(int nlines)
{
    if (access(usql_history, F_OK)) 
        write_history(usql_history);
    else if(history_length > 2 * HISTORY_ENTRY_LIMIT) {
        stifle_history(HISTORY_ENTRY_LIMIT);
        write_history(usql_history);
    }
    else
        append_history(nlines, usql_history);

    return; 
}

