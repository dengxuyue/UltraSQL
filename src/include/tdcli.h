#ifndef _TDCLIV2_H_
#define _TDCLIV2_H_

#include "config.h"

#include <coptypes.h>
#include <coperr.h>
#include <parcel.h>
#include <dbcarea.h>
#include <dbchqep.h>

#define CONNECTED        0
#define D8CAIRXSIZ       24
#define FIXED_ELM_LEN_8  42
#define D8XILMNTSIZE     4
#define SPOPTIONSSIZE    2
#define NOT_CONNECTED    1
#define OK               0
#define STOP             1
#define FAILED          -1
#define MAX_SESSIONS     1

typedef struct error_fail_t {
    Word StatementNo;
    Word Info;
    Word Code;
    Word Length;
    char Msg[255];
} error_fail;

extern char COPCLIVersion[];
extern char COPMTDPVersion[];
extern char COPMOSIosVersion[];
extern char COPMOSIDEPVersion[];
extern char OSERRVersion[];

typedef struct td_session_t {
    long     request;
    long     session;
    char     context[4];
    DBCAREA* dbcarea;
} td_session;

void tdcli_write_error(char* parcel);
void tdcli_set_options(td_session *sess);
void tdcli_set_dbcareax(td_session* sess);
unsigned int tdcli_get_segmentsize(td_session *sess);

Int16 tdcli_init(td_session *sess);
Int16 tdcli_connect(td_session *sess, char* logon);
Int16 tdcli_send_request(td_session *sess, char* req);
Int16 tdcli_end(td_session *sess);

#endif /* _TDCLIV2_H_ */
