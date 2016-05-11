#ifndef _MYCLIENT_H_
#define _MYCLIENT_H_

#include <my_global.h>
#include <mysql.h>

typedef struct mi_session_t {
    MYSQL*     connection;
    MYSQL_RES* portal;
}mi_session;


int mi_init    (mi_session* sess);
int mi_connect (mi_session* sess, char *conninfo, int infolen);
int mi_execute (mi_session* sess, char* req);
int mi_fetch   (mi_session* sess);
int mi_finish  (mi_session* sess);
int mi_end     (mi_session* sess);

void set_my_sidetitles(int on_off);
#endif
