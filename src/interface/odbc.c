#include "odbc.h"
#include "pager.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

static int interrupt_odbc_fetching = 0;
void odbc_inter_fetch (int signo)
{
    interrupt_odbc_fetching = 1;
}


/*
 * see Retrieving ODBC Diagnostics
 * for a definition of extract_error().
 */
static void extract_error( char *fn, SQLHANDLE handle, SQLSMALLINT type)
{
    SQLINTEGER   i = 0;
    SQLINTEGER   native;
    SQLCHAR      state[7];
    SQLCHAR      text[256];
    SQLSMALLINT  len;
    SQLRETURN    ret;

    fprintf(stderr,
            "\n"
            "The driver reported the following diagnostics whilst running "
            "%s\n\n",
            fn);

    do
    {
        ret = SQLGetDiagRec(type, handle, ++i, state, &native, text,
                            sizeof(text), &len );
        if (SQL_SUCCEEDED(ret))
            printf("%s:%ld:%ld:%s\n", state, i, native, text);
    }
    while( ret == SQL_SUCCESS );
}


int odbc_init (odbc_session *sess)
{
    if (!sess)
        return -1;

    sess->status = 0;
    SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sess->env);
    SQLSetEnvAttr(sess->env, SQL_ATTR_ODBC_VERSION, (void *) SQL_OV_ODBC3, 0);
    sess->status |= 1;
    SQLAllocHandle(SQL_HANDLE_DBC, sess->env, &sess->dbc);
    sess->status |= 2;

    return 0;
}


int odbc_connect(odbc_session *sess, char* logon, int length)
{
    if (!sess || !logon) return -1;

    SQLRETURN   ret;
    SQLCHAR     outstr[1024];
    SQLSMALLINT outstrlen;

    ret = SQLDriverConnect(sess->dbc, NULL, logon, SQL_NTS,
                           outstr, sizeof(outstr), &outstrlen,
                           SQL_DRIVER_COMPLETE);

    if (!SQL_SUCCEEDED(ret)) {
        fprintf(stderr, "Failed to connect.\n");
        extract_error("SQLDriverConnect", sess->dbc, SQL_HANDLE_DBC);

        return 1;
    }

    /**
     * debug info
    printf("Connected.\n");
    printf("Returned connection string was:\n\t%s\n", outstr);
    if (ret == SQL_SUCCESS_WITH_INFO) {
        printf("Driver reported the following diagnostics\n");
        extract_error("SQLDriverConnect", sess->dbc, SQL_HANDLE_DBC);
    }
    */

    SQLCHAR dbms_name[256], dbms_ver[256];
    SQLUINTEGER getdata_support;
    SQLUSMALLINT max_concur_act;
    SQLSMALLINT string_len;

    /**
     *  Find something out about the driver.
     *
     */
    SQLGetInfo(sess->dbc, SQL_DBMS_NAME, (SQLPOINTER)dbms_name,
               sizeof(dbms_name), NULL);
    SQLGetInfo(sess->dbc, SQL_DBMS_VER, (SQLPOINTER)dbms_ver,
               sizeof(dbms_ver), NULL);
    printf("DBMS Name   : %s\n", dbms_name);
    printf("DBMS Version: %s\n", dbms_ver);

    /**
     * Extra extensions
    SQLGetInfo(sess->dbc, SQL_GETDATA_EXTENSIONS, (SQLPOINTER)&getdata_support,
               0, 0);
    if (getdata_support & SQL_GD_ANY_ORDER)
        printf("SQLGetData - columns can be retrieved in any order\n");
    else
        printf("SQLGetData - columns must be retrieved in order\n");
    if (getdata_support & SQL_GD_ANY_COLUMN)
        printf("SQLGetData - can retrieve columns before last bound one\n");
    else
        printf("SQLGetData - columns must be retrieved after last bound one\n");

    SQLGetInfo(sess->dbc, SQL_MAX_CONCURRENT_ACTIVITIES, &max_concur_act, 0, 0);
    if (max_concur_act == 0) {
        printf("SQL_MAX_CONCURRENT_ACTIVITIES - no limit or undefined\n");
    } else {
        printf("SQL_MAX_CONCURRENT_ACTIVITIES = %u\n", max_concur_act);
    }
    */


    return 0;
}


int odbc_check_connection ()
{
    return 1;
}


int odbc_execute (odbc_session *sess, char* req)
{
    if (!sess || !req) return -1;

    SQLRETURN ret;
    ret = SQLAllocHandle(SQL_HANDLE_STMT, sess->dbc, &sess->stmt);
    if (!SQL_SUCCEEDED(ret)) {
        return 1;
    }

    sess->status |= 4;
    ret = SQLPrepare(sess->stmt, req, strlen(req));
    if (!SQL_SUCCEEDED(ret)) {
        /* TODO: log errors */
        extract_error("SQLPrepare", sess->dbc, SQL_HANDLE_DBC);
        return 2;
    }

    ret = SQLExecute(sess->stmt);
    if (!SQL_SUCCEEDED(ret)) {
        /* TODO: log errors */
        extract_error("SQLExecute", sess->dbc, SQL_HANDLE_DBC);
        return 3;
    }

    return 0;
}

#define IN_SCREEN(x) ((x) < output_screen_width ? 1 : 0)
int odbc_fetch (odbc_session *sess)
{
    if (!sess) return -1;
    if (!(sess->status & 4)) return 0;
    interrupt_odbc_fetching = 0;

    SQLRETURN   ret;
    SQLINTEGER  rows;
    SQLSMALLINT cols;

    SQLRowCount(sess->stmt, &rows);
    SQLNumResultCols(sess->stmt, &cols);

    printf("-*- Query has been complete. %d rows %d columns returned.\n\n",
                rows, cols);

    FILE * fout = stdout;
    if (rows > DEFAULT_SCREEN_HEIGHT) {
        open_resp_pager();
        if(internal_pager) {
            fout = internal_pager;
            signal_resp_pager(odbc_inter_fetch);
        }
    }

    /**
     * metadata of resultset
     *
     */
    SQLUSMALLINT i;
    SQLCHAR col_name[256];
    SQLSMALLINT name_length, data_type, dec_digit, nullable;
    SQLULEN col_size;

    int* col_length = (int*)malloc(sizeof(int) * cols);
    char* row_buf   = (char*)malloc(sizeof(char) * output_screen_width + 1);
    int   row_len, j, cols_in_buf, var_col_len;

    /* column names */
    row_len = 0;
    for (i = 1; i <= cols; i++) {
        if (IN_SCREEN(row_len))
            cols_in_buf = i;

        col_length[i - 1] = 0;
        ret = SQLDescribeCol( sess->stmt,
                              i,
                              col_name, 256,
                              &name_length,
                              &data_type,
                              &col_size,
                              &dec_digit,
                              &nullable );

        if(i > 1 && IN_SCREEN(row_len)) {
            sprintf(row_buf + row_len, "%s", "|");
            row_len++;
        }

        if (SQL_SUCCEEDED(ret)) {
            if(IN_SCREEN(row_len)) {
                /* we have an ending '\0' */
                var_col_len = snprintf(row_buf + row_len, output_screen_width - row_len + 1, "%s", col_name);
                if (var_col_len > output_screen_width - row_len)
                    row_len = output_screen_width;
                else
                    row_len += var_col_len;
            }

            if (col_size > name_length) {
                for(j = 0; j < col_size - name_length; j++)
                    if (IN_SCREEN(row_len)) {
                        sprintf(row_buf + row_len, "%s", " ");
                        row_len++;
                    }

                col_length[i - 1] = col_size;
            }
            else {
                col_length[i - 1] = name_length;
            }
        }

    }
    row_buf[row_len] = '\0';
    fprintf(fout, row_buf);
    fprintf(fout, "\n");

    /* delimiter ------+------ */
    row_len = 0;
    for (i = 1; i <= cols; i++) {
        if(i > 1 && IN_SCREEN(row_len)) {
            sprintf(row_buf + row_len, "%s", "+");
            row_len++;
        }
        for (j = 0; j < col_length[i - 1]; j++) {
            if (IN_SCREEN(row_len)) {
                sprintf(row_buf + row_len, "%s", "-");
                row_len++;
            }
        }
    }
    row_buf[row_len] = '\0';
    fprintf(fout, row_buf);
    fprintf(fout, "\n");

    while (SQL_SUCCEEDED(ret = SQLFetch(sess->stmt))) {
        row_len = 0;
        for (i = 1; i <= cols_in_buf; i++) {
            SQLINTEGER indicator;
            char buf[512];
            ret = SQLGetData(sess->stmt, i, SQL_C_CHAR,
                             buf, sizeof(buf), &indicator);
            if(i > 1 && IN_SCREEN(row_len)) {
                sprintf(row_buf + row_len, "%s", "|");
                row_len++;
            }
            if (SQL_SUCCEEDED(ret)) {
                if (indicator == SQL_NULL_DATA) strcpy(buf, "NULL");

                if (IN_SCREEN(row_len)) {
                    /* we have an ending '\0' */
                    var_col_len = snprintf(row_buf + row_len, output_screen_width - row_len + 1, "%s", buf);
                    if (var_col_len > output_screen_width - row_len)
                        row_len = output_screen_width;
                    else
                        row_len += var_col_len;
                }

                for(j = 0; j < col_length[i - 1] - strlen(buf); j++)
                    if(IN_SCREEN(row_len)) {
                        sprintf(row_buf + row_len, "%s", " ");
                        row_len++;
                    }
            }
        }
        row_buf[row_len] = '\0';
        fprintf(fout, row_buf);
        fprintf(fout, "\n");

        if (interrupt_odbc_fetching)
            break;
    }
    fprintf(fout, "\n");

    free(col_length);
    free(row_buf);
    signal_resp_pager(SIG_IGN);
    close_resp_pager();

    return 0;
}


int odbc_finish (odbc_session *sess)
{
    if (!sess) return -1;
    if (!(sess->status & 4)) return 0;

    SQLRETURN   ret;

    SQLFreeHandle(SQL_HANDLE_STMT, sess->stmt);
    sess->status = sess->status & ~4;

    return 0;
}


int odbc_end (odbc_session *sess)
{
    if (!sess) return -1;

    SQLRETURN   ret;

    SQLDisconnect(sess->dbc);
    SQLFreeHandle(SQL_HANDLE_DBC, sess->dbc);
    SQLFreeHandle(SQL_HANDLE_ENV, sess->env);
    sess->status = 0;

    return 0;
}
