#ifndef _FORMAT_H_
#define _FORMAT_H_

#include "config.h"
#include "helper.h"

#define RESPONSE_TITLESTART    1
#define RESPONSE_TITLE         2
#define RESPONSE_FORMAT        3
#define RESPONSE_DATA          4
#define RESPONSE_DATA_REGULAR  41
#define RESPONSE_DATA_SHOWDML  42
#define RESPONSE_RECEND        5


typedef struct column_headline_t {
    int col_count;
    int col_index;
    int *col_length;
    int block[BUFFER_HEADLINE_BLOCK];
} column_headline;

typedef struct column_response_t {
    int type;
    int datatype;
    int length;
    union {
        char* title;
        char* format;
        char* data;
    } column;

    /* pre-alloc block for small parcel data*/
    char block[BUFFER_PARCEL_BLOCK];
} column_response;

ts_sigfunc sigal_resp_pager (ts_sigfunc int_pager);

void open_resp_pager();
void close_resp_pager();
int init_resp_buffer();
void fini_resp_buffer();
void reset_resp_buffer(int col_length);

int add_resp_buffer(int type, int datatype, int length, void* column);
int flush_resp_buffer();

#endif
