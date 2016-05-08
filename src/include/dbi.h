#ifndef _DBI_H_
#define _DBI_H_

#include "config.h"
#include "node.h"

#define TDCLI_SESSION  1
#define ODBC_SESSION   2
#define PQLIB_SESSION  3
#define MYSQL_SESSION  4


typedef struct connection_session_t {
    int   type;
    char  alias[CONN_SESSION_ALIAS_LENGTH];
    char  conn_info[CONN_INFO_STRING_LENGTH];
    void* session;
}conn_session;


void dbi_show_session (char* alias);
void dbi_preinit_all ();
int  dbi_init (int protocal);
int  dbi_connect (char* logon, int length);
int  dbi_execute (char *req);
int  dbi_fetch ();
int  dbi_finish ();
int  dbi_end ();
int  dbi_end_all ();
int  dbi_deinit_all ();

int  adjust_active_session_index (char *alias);
void enable_multi_session ();
void set_response_sidetitles(int on_off);

#endif /* _DBI_H_ */
