#include "profile.h"
#include <string.h>


static int dbs_user_passwds = -1;
static dbs_passwd dbs_user_passwd[PASSWORD_ENTRY_LIMIT];


int add_dbs_passwd (char* line)
{
    if (dbs_user_passwds + 1 > PASSWORD_ENTRY_LIMIT - 1)
        return 1;

    int start, i;
    int line_len = strlen(line);

    /*exclude CRLF if any*/
    while (line[line_len - 1] == '\n' || line[line_len - 1] == '\r')
        line_len--;

    start = i = 0;
    for(; i < line_len; i++) {
        if (line[i] == '|' || line[i] == '\0') {
            if(i == start)
                return -1;
            strncpy(dbs_user_passwd[dbs_user_passwds + 1].datasource, line + start, i - start);
            dbs_user_passwd[dbs_user_passwds + 1].datasource[i - start] = '\0';
            break;
        }
    }

    i++;
    start = i;
    for(; i < line_len; i++) {
        if (line[i] == '|' || line[i] == '\0') {
            if(i == start)
                return -1;
            strncpy(dbs_user_passwd[dbs_user_passwds + 1].username, line + start, i - start);
            dbs_user_passwd[dbs_user_passwds + 1].username[i - start] = '\0';
            break;
        }
    }

    i++;
    start = i;
    if(start == line_len -1)
        return -1;
    strncpy(dbs_user_passwd[dbs_user_passwds + 1].password, line + start, line_len - start);
    dbs_user_passwd[dbs_user_passwds + 1].password[line_len - start] = '\0';

    dbs_user_passwds++;
    return 0;
}


char* get_password (char* dbs, char* user)
{
    int i;
    for(i = 0; i <= dbs_user_passwds; i++) {
        if(!(strcmp(dbs, dbs_user_passwd[i].datasource) || strcmp(user, dbs_user_passwd[i].username)))
            return dbs_user_passwd[i].password;
    }

    return 0;
}

