#ifndef _PROFILE_H_
#define _PROFILE_H_

#include "config.h"

typedef struct dbs_passwd_t {
    char datasource[64];
    char username[32];
    char password[32];
} dbs_passwd;

int  init_profile();
void fini_profile();

int   add_dbs_passwd(char* line);
char* get_password(char* dbs, char* user);
int   append_setting_list (char* line);
char* get_next_setting_item ();

#endif
