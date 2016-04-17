#ifndef _PQLIB_H_
#define _PQLIB_H_

#include <libpq-fe.h>

typedef struct pq_session_t {
    char      def_pg_port[8];
    PGconn*   connection;
    PGresult* portal;
}pq_session;


int pq_init    (pq_session* sess);
int pq_connect (pq_session* sess, char *conninfo, int infolen);
int pq_execute (pq_session* sess, char* req);
int pq_fetch   (pq_session* sess);
int pq_finish  (pq_session* sess);
int pq_end     (pq_session* sess);

#endif
