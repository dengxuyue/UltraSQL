/*
 * summary: communication module with Teradata
 *
 * Status:
 *     No big feature is underlying development
 */
#include "tdcli.h"
#include "parcels.h"
#include "pager.h"
#include "debug.h"
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>

int interrupt_tdcli_fetching = 0;
struct timeval request_beginning = {
    .tv_sec = 0,
    .tv_usec =0
};

char byte_inter_ptrs[] = {
    (char)127, /* ASCII */
    (char)63   /* UTF-8 */
};

void tdcli_inter_fetch (int signo)
{
    interrupt_tdcli_fetching = 1;
}


void tdcli_write_error(char *parcel)
{
    int i;
    error_fail* error = (error_fail *) (parcel);
    fprintf(stdout, "-!- Errors occur while executing Statment No.%d.\n", error->StatementNo);
    fprintf(stdout, "-!- Code   : %d\n", error->Code);
    fprintf(stdout, "-!- Message: ");
    for(i = 0; i < (int)error->Length; i++)
        printf("%c", error->Msg[i]);
    printf("\n\n");
    return;
}


Int16 tdcli_init(td_session *sess)
{
    INTERFACE_DEBUG_C();

    Int32 result;
    sess->dbcarea = (DBCAREA*)malloc(sizeof(DBCAREA));
    sess->dbcarea->total_len = sizeof(DBCAREA);

    DBCHINI(&result, sess->context, sess->dbcarea);
    if (result != EM_OK) {
        fprintf(stdout, "-!- Error in initialization for TDCLIv2: %s\n",sess->dbcarea->msg_text);
        tdcli_end(sess);
        return -1;
    }

    tdcli_set_options(sess);
    /* tdcli_set_dbcareax(sess); */

    fprintf(stdout, "\nCLIv2   version is %s\n",   COPCLIVersion);
    fprintf(stdout,   "MTDP    version is %s\n",   COPMTDPVersion);
    fprintf(stdout,   "MOSIOS  version is %s\n",   COPMOSIosVersion);
    fprintf(stdout,   "MOSIDEP version is %s\n",   COPMOSIDEPVersion);
    fprintf(stdout,   "OSERR   version is %s\n\n", OSERRVersion);

    return EM_OK;
}


Int16 tdcli_connect(td_session* sess, char* logon)
{
    INTERFACE_DEBUG_C();
    INTERFACE_DEBUG("Logging on to: %s", logon);

    Int32 result;
    gettimeofday(&request_beginning, 0);

    sess->dbcarea->logon_ptr=logon;
    sess->dbcarea->logon_len=strlen(logon);
    sess->dbcarea->func=DBFCON;
    DBCHCL(&result, sess->context, sess->dbcarea);
    if (result != EM_OK) {
        fprintf(stdout, "-!- Error in Logon: %s\n", sess->dbcarea->msg_text);
        tdcli_end(sess);
        return -1;
    }
    sess->request = sess->dbcarea->o_req_id;
    sess->session = sess->dbcarea->o_sess_id;

    return EM_OK;
}


Int16 tdcli_send_request(td_session* sess, char *req)
{
    INTERFACE_DEBUG_C();
    INTERFACE_DEBUG("Submitting request: %s", req);

    Int32 result;
    sess->dbcarea->func = DBFIRQ;
    gettimeofday(&request_beginning, 0);

    sess->dbcarea->req_ptr = req;
    sess->dbcarea->req_len = strlen(req);
    DBCHCL(&result, sess->context, sess->dbcarea);
    if (result != EM_OK) {
        fprintf(stderr, "-!- Error in Initiating a request: %s\n", sess->dbcarea->msg_text);
        tdcli_end(sess);
        return -1;
    }
    sess->request = sess->dbcarea->o_req_id;
    sess->session = sess->dbcarea->o_sess_id;
    interrupt_tdcli_fetching = 0;

    INTERFACE_DEBUG_S("Submitting request successed");

    return EM_OK;
}


static inline void row_column_detail(int row, int col, char *act)
{
    printf(" %d rows", row);
    if(row && col)
        printf(" %d columns", col);
    printf(" %s.\n\n", act);
    return;
}

/*
 * Find ActivityType in </usr/include/parcel.h>
 */
static void resp_count(Word ActivityType, UInt32 ActivityCount, Word FieldCount)
{
    double elapsed_time = .0;
    struct timeval now;
    gettimeofday(&now, 0);
    if (request_beginning.tv_sec || request_beginning.tv_usec) {
        elapsed_time = now.tv_sec - request_beginning.tv_sec +
            0.001 * (now.tv_usec - request_beginning.tv_usec) * 0.001;

        request_beginning.tv_sec  = 0;
        request_beginning.tv_usec = 0;
    }
    printf("-*- Total elapsed time is %.5f seconds.\n", elapsed_time);

    if (ActivityCount > DEFAULT_SCREEN_HEIGHT &&
        (ActivityType == PclRetStmt ||
         0)) {
        open_resp_pager();
        signal_resp_pager(tdcli_inter_fetch);
    }

    switch(ActivityType){
    case PclStmtNull:       /* 0 */
        break;
    case PclRetStmt:        /* 1 */
        printf("-*- Query has been completed.");
        row_column_detail(ActivityCount, FieldCount, "returned");
        break;
    case PclInsStmt:        /* 2 */
        printf("-*- Insert has been completed.");
        row_column_detail(ActivityCount, FieldCount, "added");
        break;
    case PclUpdStmt:        /* 3 */
    case PclUpdRetStmt:     /* 4 */
        printf("-*- Update has been completed.");
        row_column_detail(ActivityCount, FieldCount, "changed");
        break;
    case PclDelStmt:        /* 5 */
        printf("-*- Table has been deleted.");
        row_column_detail(ActivityCount, FieldCount, "deleted");
        break;
    case PclCTStmt:         /* 6 */
        printf("-*- Table has been created.\n\n");
        break;
    case PclModTabStmt:     /* 7 */
        printf("-*- Table has been modified.\n\n");
        break;
    case PclCVStmt:         /* 8 */
        printf("-*- View has been created.\n\n");
        break;
    case PclCMStmt:         /* 9 */
        printf("-*- Macro has been created.\n\n");
        break;
    case PclDropTabStmt:    /* 10 */
        printf("-*- Table has been dropped. %d rows deleted\n\n",
                ActivityCount);
        break;
    case PclDropViewStmt:   /* 11 */
        printf("-*- View has been dropped.\n\n");
        break;
    case PclDropMacStmt:    /* 12 */
        printf("-*- Macro has been dropped.\n\n");
        break;
    case PclDropIndStmt:    /* 13 */
        printf("-*- Index has been dropped. %d rows deleted\n\n",
                ActivityCount);
        break;
    case PclRenTabStmt:     /* 14 */
        printf("-*- Table has been renamed.\n\n");
        break;
    case PclRenViewStmt:    /* 15 */
        printf("-*- View has been renamed.\n\n");
        break;
    case PclRenMacStmt:     /* 16 */
        printf("-*- Macro has been renamed.\n\n");
        break;
    case PclCreIndStmt:     /* 17 */
        printf("-*- Index has been created.\n\n");
        break;
    case PclCDStmt:         /* 18 */
        printf("-*- Database has been created.\n\n");
        break;
    case PclCreUserStmt:    /* 19 */
        printf("-*- User has been created.\n\n");
        break;
    case PclGrantStmt:      /* 20 */
        printf("-*- Grant has been accepted.\n\n");
        break;
    case PclRevokeStmt:     /* 21 */
        printf("-*- Revoke has been accepted.\n\n");
        break;
    case PclGiveStmt:       /* 22 */
        printf("-*- Revoke has been accepted.\n\n");
        break;
    case PclDropDBStmt:     /* 23 */
        printf("-*- Database has been dropped. %d rows deleted.\n\n",
                ActivityCount);
        break;
    case PclModDBStmt:      /* 24 */
        printf("-*- Database has been modified.\n\n");
        break;
    case PclDatabaseStmt:   /* 25 */
        printf("-*- Default database has been set.\n\n");
        break;
    case PclBTStmt:         /* 26 */
        printf("-*- BT/ET transaction command has been received.\n\n");
        break;
    case PclETStmt:         /* 27 */
        printf("-*- BT/ET transaction command has been received.\n\n");
        break;
    case PclAbortStmt:      /* 28 */
        printf("-*- Abort transaction command has been received.\n\n");
        break;
    case PclNullStmt:       /* 29 */
        printf("-*- Null request has been received.\n\n");
        break;
    case PclRepMacStmt:     /* 35 */
        printf("-*- Macro has been replaced.");
        row_column_detail(ActivityCount, FieldCount, "returned");
        break;
    case PclCheckPtStmt:    /* 36 */
        printf("-*- Checkpoint has been made.");
        row_column_detail(ActivityCount, FieldCount, "returned");
        break;
    case PclDelDBStmt:      /* 46 */
        printf("-*- Database has been deleted.\n\n");
        break;
    case PclShowStmt:       /* 49 */
        printf("-*- Show table/view/procedure successful.\n\n");
        break;
    case PclHelpStmt:       /* 50 */
        printf("-*- Help/Explain information retrieved. %d rows returned.\n\n",
            ActivityCount);
        break;
    case PclDropProcStmt:   /* 103 */
        printf("-*- Procedure has been dropped.\n\n");
        break;
    case PclCreateProcStmt: /* 104 */
        printf("-*- Procedure has been created.\n\n");
        break;
    case PclCallStmt:       /* 105 */
        printf("-*- Procedure has been executed.\n\n");
        break;
    case PclRenProcStmt:    /* 106 */
        printf("-*- Procedure has been renamed.\n\n");
        break;
    case PclRepProcStmt:    /* 107 */
        printf("-*- Procedure has been replaced.\n\n");
        break;
    case PclBeginDBQLStmt:  /* 112 */
        printf("-*- Begin querylog accepted.\n\n");
        break;
    case PclEndDBQLStmt:    /* 113 */
        printf("-*- End querylog accepted.\n\n");
        break;
    case PclMrgMixedStmt:   /* 127 */
        printf("-*- Merge(upsert) has been completed.");
        row_column_detail(ActivityCount, FieldCount, "changed");
        break;
    case PclMrgUpdStmt:     /* 128 */
        printf("-*- Merge(update) has been completed.");
        row_column_detail(ActivityCount, FieldCount, "updated");
        break;
    case PclMrgInsStmt:     /* 129 */
        printf("-*- Merge(insert) has been completed.");
        row_column_detail(ActivityCount, FieldCount, "added");
        break;
    case PclSetQBandStmt:     /* 165 */
        printf("-*- Set QUERY_BAND has been accepted.\n\n");
        break;
    default:
        printf("-*- Unknown type (%u) request accepted. %d rows %d columns returned.\n\n",
            ActivityType, ActivityCount, FieldCount);
        break;
    }

    return;
}


Int16 tdcli_fetch_request(td_session *sess)
{
    INTERFACE_DEBUG_C();
    Int32 result;
    int response_type = 0, showflag = 0;
    int retval_resp_buf;
    int data_type;
    int i, status;
    char ctc[6400];
    struct CliSuccessType *SuccPcl;
    struct CliFailureType *FailPcl;
    struct CliOkType *OKPcl;

    sess->dbcarea->i_sess_id = sess->session;
    sess->dbcarea->i_req_id  = sess->request;
    sess->dbcarea->func = DBFFET;

    status = OK;
    while (status == OK && !interrupt_tdcli_fetching) {
        DBCHCL(&result,sess->context,sess->dbcarea);
        if (result == REQEXHAUST)
            status = STOP;
        else if (result != EM_OK)
            status = FAILED;
        else {
            INTERFACE_DEBUG("Flavor Type: %d", sess->dbcarea->fet_parcel_flavor);

            switch (sess->dbcarea->fet_parcel_flavor) {
            case PclSUCCESS: /* 8 */
                SuccPcl = (struct CliSuccessType *) sess->dbcarea->fet_data_ptr;

                INTERFACE_DEBUG("Success parcel, length %d", (Int32)sess->dbcarea->fet_ret_data_len);
                INTERFACE_DEBUG("Activity Type : %d", SuccPcl->ActivityType);
                INTERFACE_DEBUG("Activity Count: %d", SuccPcl->ActivityCount);

                resp_count(SuccPcl->ActivityType, 0, 0);
                break;

            case PclFAILURE: /* 9 */
                INTERFACE_DEBUG("Failure parcel, length %d", (Int32)sess->dbcarea->fet_ret_data_len);
                tdcli_write_error(sess->dbcarea->fet_data_ptr);
                break;

            case PclRECORD : /* 10 */
                INTERFACE_DEBUG("Record parcel, length %d", (Int32)sess->dbcarea->fet_ret_data_len);
                break;

            case PclENDSTATEMENT: /* 11 */
                INTERFACE_DEBUG_S("EndStatement parcel");
                flush_resp_buffer();
                reset_resp_buffer(0);
                break;

            case PclENDREQUEST: /* 12 */
                INTERFACE_DEBUG_S("EndRequest parcel");
                break;

            case PclOK: /* 17 */
                OKPcl = (struct CliOkType *) sess->dbcarea->fet_data_ptr;

                INTERFACE_DEBUG_S("OK parcel");
                INTERFACE_DEBUG("Activity Type : %d", OKPcl->ActivityType);
                INTERFACE_DEBUG("Activity Count: %d",OKPcl->ActivityCount);

                resp_count(OKPcl->ActivityType, OKPcl->ActivityCount, OKPcl->FieldCount);
                reset_resp_buffer(OKPcl->FieldCount);
                if (OKPcl->ActivityType == PclShowStmt)
                    showflag = 1;
                else
                    showflag = 0;

                break;

            case PclFIELD: /* 18 */
                INTERFACE_DEBUG("Field parcel, length %d", (Int32)sess->dbcarea->fet_ret_data_len);
                if(DEBUG_ON & 0xF0) {
                    printf("----------- %d -----------------\n", response_type);
                    for (i = 0; i < sess->dbcarea->fet_ret_data_len; i++)
                        printf("%c", sess->dbcarea->fet_data_ptr[i]);
                    printf("\n");
                }
                if (response_type == RESPONSE_TITLE ||
                    response_type == RESPONSE_FORMAT ||
                    response_type == RESPONSE_RECEND)
                    data_type = 0;
                else if (response_type == RESPONSE_DATA) {
                    if (showflag)
                        data_type = RESPONSE_DATA_SHOWDML;
                    else
                        data_type = RESPONSE_DATA_REGULAR;
                }
                else {
                    data_type = 0;
                }
                retval_resp_buf = add_resp_buffer(response_type,
                                                 data_type,
                                                 sess->dbcarea->fet_ret_data_len,
                                                 sess->dbcarea->fet_data_ptr);
                if(retval_resp_buf < 0)
                    interrupt_tdcli_fetching = 1;

                break;

            case PclNULLFIELD: /* 19 */
                INTERFACE_DEBUG_S("NullField parcel");
                add_resp_buffer(response_type, data_type, 0, 0);
                break;

            case PclTITLESTART: /* 20 */
                INTERFACE_DEBUG_S("TitleStart parcel");
                response_type = RESPONSE_TITLE;
                break;

            case PclTITLEEND: /* 21 */
                INTERFACE_DEBUG_S("TitleEnd parcel");
                response_type = 0;
                break;

            case PclFORMATSTART: /* 22 */
                INTERFACE_DEBUG_S("FormatStart parcel");
                response_type = RESPONSE_FORMAT;

                break;

            case PclFORMATEND: /* 23 */
                INTERFACE_DEBUG_S("FormatEnd parcel");
                response_type = 0;

            case PclSIZESTART: /* 24 */
                INTERFACE_DEBUG_S("SizeStart parcel");
                break;

            case PclSIZEEND: /* 25 */
                INTERFACE_DEBUG_S("SizeEnd parcel");
                break;

            case PclSIZE: /* 26 */
                INTERFACE_DEBUG_S("Size parcel");
                break;

            case PclRECSTART: /* 27 */
                INTERFACE_DEBUG_S("RecordStart parcel");
                response_type = RESPONSE_DATA;

                break;

            case PclRECEND: /* 28 */
                INTERFACE_DEBUG_S("RecordEnd parcel");
                add_resp_buffer(RESPONSE_RECEND, 0, 0, 0);
                response_type = 0;

                break;

            case PclNOP: /* 32 */
                INTERFACE_DEBUG_S("Nop parcel");
                break;

            case PclWITH: /* 33 */
                INTERFACE_DEBUG_S("With parcel");
                break;

            case PclPOSITION: /* 34 */
                INTERFACE_DEBUG_S("Position parcel");
                break;

            case PclENDWITH: /* 35 */
                INTERFACE_DEBUG_S("EndWidth parcel");
                break;

            case PclPOSSTART: /* 46 */
                INTERFACE_DEBUG_S("PosStart parcel");
                break;

            case PclPOSEND: /* 47 */
                INTERFACE_DEBUG_S("PosEnd parcel");
                break;

            case PclERROR: /* 49 */
                INTERFACE_DEBUG_S("ERROR parcel");
                status = STOP;
                tdcli_write_error(sess->dbcarea->fet_data_ptr);
                tdcli_end(sess);
                return -1;

            case PclDATAINFO: /* 71 */
                INTERFACE_DEBUG_S("DataInfo parcel");
                break;

            case PclPREPINFO: /* 86 */
                INTERFACE_DEBUG_S("PrepInfo parcel");
                break;

            case PclASSIGNRSP: /* 101 */
                INTERFACE_DEBUG_S("AssignRsp parcel");
                break;

            case PclCURSORDBC: /* 121 */
                INTERFACE_DEBUG_S("CursorDBC parcel");
                break;

            case PclFLAGGER: /* 122 */
                INTERFACE_DEBUG_S("Flagger parcel");
                break;

            case PclERRORINFO: /* 164 */
                INTERFACE_DEBUG_S("ErrorInfo parcel");
                break;

            case PclSTATEMENTINFO: /* 169 */
                INTERFACE_DEBUG_S("StatementInfo parcel");
                break;

            case PclSTATEMENTINFOEND: /* 170 */
                INTERFACE_DEBUG_S("StatementInfoEnd parcel");
                break;

            case PclRESULTSET: /* 172 */
                INTERFACE_DEBUG_S("ResultSet parcel");
                break;

            default:
                INTERFACE_DEBUG_S("Unprocessed parcel");
                break;

            } /*end of switch*/
        } /*end of else*/
    } /*end of while*/

    signal_resp_pager(SIG_IGN);
    close_resp_pager();

    if(status == FAILED) {
        fprintf(stderr, "Fetch failed: %s\n",sess->dbcarea->msg_text);
        tdcli_end(sess);
        return -1;
    }

    return EM_OK;
}


Int16 tdcli_end_request(td_session *sess)
{
    Int32 result;
    interrupt_tdcli_fetching = 0;
    sess->dbcarea->i_sess_id = sess->dbcarea->o_sess_id;
    sess->dbcarea->i_req_id  = sess->dbcarea->o_req_id;
    sess->dbcarea->func = DBFERQ;
    DBCHCL(&result, sess->context, sess->dbcarea);
    if (result != EM_OK){
        printf("Error in EndRequest: %s\n",sess->dbcarea->msg_text);
        tdcli_end(sess);
        return -1;
    }
    return EM_OK;
}


Int16 tdcli_end(td_session* sess)
{
    Int32 result;
    /* tdcli_clear_dbcareax(sess); */
    sess->dbcarea->func = DBFDSC;
    DBCHCL(&result,sess->context, sess->dbcarea);
    DBCHCLN(&result,sess->context);

    INTERFACE_DEBUG("Exiting: %s", sess->dbcarea->msg_text);

    return 0;
}


void tdcli_set_options(td_session* sess)
{
    sess->dbcarea->change_opts       = 'Y';
    sess->dbcarea->charset_type      = 'C';
    sess->dbcarea->keep_resp         = 'N';
    sess->dbcarea->inter_ptr         = &byte_inter_ptrs[0];
    sess->dbcarea->loc_mode          = 'Y';
    sess->dbcarea->parcel_mode       = 'Y';
    sess->dbcarea->resp_mode         = 'F';
    sess->dbcarea->req_buf_len       = 1024;
    sess->dbcarea->req_proc_opt      = 'E';
    sess->dbcarea->request_mode      = 'P';
    sess->dbcarea->resp_buf_len      = 8196;
    sess->dbcarea->ret_time          = 'N';
    sess->dbcarea->save_resp_buf     = 'N';
    sess->dbcarea->tell_about_crash  = 'Y';
    sess->dbcarea->two_resp_bufs     = 'N';
    sess->dbcarea->tx_semantics      = 'D';
    sess->dbcarea->use_presence_bits = 'N';
    sess->dbcarea->var_len_req       = 'N';
    sess->dbcarea->var_len_fetch     = 'N';
    sess->dbcarea->wait_across_crash = 'N';
    sess->dbcarea->wait_for_resp     = 'Y';

    /*
     * Turn on the following flags to fix Bug 8.
     *
     * sess->dbcarea->return_object   = 'D';
     */
    sess->dbcarea->consider_APH_resps = 'Y';

    /**
    sess->dbcarea->columnInfo        = 'O';
    sess->dbcarea->connect_type      = 'C';
    sess->dbcarea->lang_conformance  = 'N';
    sess->dbcarea->maximum_parcel    = 'O';
    sess->dbcarea->sess_2pc_mode     = 'N';

    sess->dbcarea->max_decimal_returned = 0;
    */
}


unsigned int tdcli_get_segmentsize(td_session *sess)
{
    Int32 result;
    DBCHQEP QEPParms ;
    unsigned int MaxSegSize;
    memset(&QEPParms, 0, sizeof(QEPParms));
    QEPParms.qepLevel = QEPLEVEL;
    QEPParms.qepItem  = QEPIDMSS ;
    QEPParms.qepRALen = sizeof(long);
    QEPParms.qepRArea = &MaxSegSize;
    QEPParms.qepTDP   = (long) ("nag2n1");
    QEPParms.qepTLen  = strlen("nag2n1");
    DBCHQE(&result, sess->context, &QEPParms);
    MaxSegSize = *(unsigned int *) QEPParms.qepRArea;
    printf(" MaxSegSize :%d", MaxSegSize);
    return(MaxSegSize);
}


void tdcli_clear_dbcareax(td_session* sess)
{
    DBCAREA *dbcarea = sess->dbcarea;
    dbcarea->dbriSeg='L';
    free(dbcarea->extension_pointer);
    dbcarea->dbriSeg='N';
}


/*
 * /usr/include/dbcarea.h
 */
void tdcli_set_dbcareax(td_session* sess)
{
    char *ext_ptr;
    D8CAIRX dbcxptr;
    D8XILMNT dbxilmnt;
    D8XILPTR dbxilptr;
    char* SplFlag = "PY";
    int temp;

    DBCAREA *dbcarea = sess->dbcarea;
    dbcarea->dbriSeg='L';

    /* Initialize the extension area in DBCAREA */
    ext_ptr = (char *)(malloc(sizeof(D8CAIRX) +
                              sizeof(struct D8XILMNT) + sizeof(D8XILPTR)));
    dbcarea->extension_pointer = (char *)ext_ptr;

    /* Set the Eyecatcher */
    strncpy(dbcxptr.d8xiId, "IRX8", 4);

    /* Set the first reserved field */
#if defined(_LP64) || defined(__LP64__) || defined(__64BIT__)
    memset(dbcxptr.d8xiNxt4, 0, 4);
#else
    dbcxptr.d8xiNext = NULL;
#endif

    /* Set Length of DBCAREA extension to */
    /* size of Extension header + Size of Extension Elements */
    dbcxptr.d8xiSize = D8CAIRXSIZ + FIXED_ELM_LEN_8;

    /* set level to zero to indicate use of 64-bit */
    /* compatible elements of IRX8 */
    dbcxptr.d8xiLvl = D8XIL64;
    /* Set the second reserved field */
    memset(dbcxptr.d8xiFil1, 0, 3);
#if defined(_LP64) || defined(__LP64__) || defined(__64BIT__)
    dbcxptr.d8xiNext = NULL;
#else
    memset(dbcxptr.d8xiNxt8, 0, 8);
#endif

    /* Place the extension header at begining of extension_pointer */
    memcpy(ext_ptr, (char*)(&dbcxptr), sizeof(D8CAIRX));
    /* Move to the end of Extension header */
    ext_ptr = (char*)ext_ptr + D8CAIRXSIZ;
    /* Build the element */
    /* Initialize the SPOptions parcel header for TDSP */
    dbxilmnt.d8xilTyp = PclSPOPTIONSTYPE;
    dbxilmnt.d8xilLen = FIXED_ELM_LEN_8;
    /* Set the high end bit of the element type to 1 */
    /* inorder to notify CLI that app, uses pointer type element */
    dbxilmnt.d8xilTyp = (dbxilmnt.d8xilTyp | 0x8000);
    memcpy(ext_ptr, (char*)(&dbxilmnt), D8XILMNTSIZE);
    ext_ptr = (char*)ext_ptr + D8XILMNTSIZE;
    /* Initialize the pointer Element */

    /* Set first reserved field to zero */
    memset(dbxilptr.d8xilpF2, 0, 2 * sizeof(Byte));
    memcpy(ext_ptr, &dbxilptr.d8xilpF2, 2 * sizeof(Byte));
    ext_ptr = (char *)ext_ptr + 2 * sizeof(Byte);

#if defined(_LP64) || defined(__LP64__) || defined(__64BIT__)
    /* Set second reserved field to zero */
    memset(dbxilptr.d8xilpP4, 0, 4 * sizeof(Byte));
    memcpy(ext_ptr, &dbxilptr.d8xilpP4, 4 * sizeof(Byte));
    ext_ptr = (char *)ext_ptr + 4 * sizeof(Byte);
#else
    /* Set address of the parcel body */
    temp = (Int32)SplFlag;
    memcpy(dbxilptr.d8xilpPt, &temp, 4);
    memcpy(ext_ptr, &dbxilptr.d8xilpPt, 4 * sizeof(char));
    ext_ptr = (char *)ext_ptr + 4 * sizeof(char);
#endif

    /* Set third reserved field to zero */
    memset(dbxilptr.d8xilpF3, 0, 4 * sizeof(UInt32));
    memcpy(ext_ptr, &dbxilptr.d8xilpF3, 4 * sizeof(UInt32));
    ext_ptr = (char *)ext_ptr + 4 * sizeof(UInt32);

    /* Set Parcel Body Length field */
    dbxilptr.d8xilpLn = SPOPTIONSSIZE;
    memcpy(ext_ptr, &dbxilptr.d8xilpLn, sizeof(UInt32));
    ext_ptr = (char *)ext_ptr + sizeof(UInt32);

    /* Set Fourth reserved field to zero */
    memset(dbxilptr.d8xilpA, 0, 4 * sizeof(Byte));
    memcpy(ext_ptr, &dbxilptr.d8xilpA, 4 * sizeof(Byte));
    ext_ptr = (char *)ext_ptr + 4 * sizeof(Byte);

#if defined(_LP64) || defined(__LP64__) || defined(__64BIT__)
    /* Set address of the parcel body */
    memcpy(dbxilptr.d8xilpPt, &SplFlag, 8);
    memcpy(ext_ptr, &dbxilptr.d8xilpPt, 8 * sizeof(char));
    ext_ptr = (char *)ext_ptr + 8 * sizeof(char);
#else
    /* Set reserved field to zero */
    memset(dbxilptr.d8xilpP8, 0, 8 * sizeof(Byte));
    memcpy(ext_ptr, &dbxilptr.d8xilpP8, 8 * sizeof(Byte));
    ext_ptr = (char *)ext_ptr + 8 * sizeof(Byte);
#endif
}
