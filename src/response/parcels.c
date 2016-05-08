/*
 * summary: display module for output from Teradata
 *
 * Status:
 *    'sidetitles on' feature (pending, no avalable DBS)
 */
#include "parcels.h"
#include "pager.h"
#include "helper.h"
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>

/* response column list */
static column_response * col_resp_list = 0;

/* the last column index which is received */
static int last_col_resp = 0;

/* column length for headline */
static column_headline col_resp_headline;

/* don't print result mode */
static int internal_quiescent_mode = 0;

/* how many character has been printed in current row */
static int internal_row_width = 0;

/* the index of title/format/data column in response */
static int title_col_index = -1;
static int data_col_index = -1;

/* More than one column are returned in the result set*/
static int multi_col_title  = 0;
static int multi_col_format = 0;
static int multi_col_data   = 0;

/* flag shows whether there're any data in the col_resp_list */
static int data_in_resp_list = 0;


int init_resp_buffer ()
{
    col_resp_headline.col_count = 0;
    col_resp_headline.col_index = 0;
    if (col_resp_list)
        return 1;
    col_resp_list = (column_response*) malloc(BUFFER_PARCELS_LIMIT * sizeof(column_response));
    if(!col_resp_list)
        return -1;

    reset_resp_buffer(0);

    return 0;
}


void reset_resp_buffer (int col_count)
{
    title_col_index = data_col_index = -1;
    multi_col_title = multi_col_format = multi_col_data = 0;
    internal_quiescent_mode = 0;
    internal_row_width = 0;
    data_in_resp_list = 0;
    int i;
    if (last_col_resp > 0) {
        for(i = 0; i < last_col_resp; i++) {
            column_response *ptr = &col_resp_list[i];
            if (ptr->length > BUFFER_PARCEL_BLOCK) {
                if (ptr->type == RESPONSE_TITLE)
                    free(ptr->column.title);
                else if (ptr->type == RESPONSE_FORMAT)
                    free(ptr->column.format);
                else if (ptr->type == RESPONSE_DATA)
                    free(ptr->column.data);
            }
        }
    }
    last_col_resp = 0;

    if (col_resp_headline.col_count > BUFFER_HEADLINE_BLOCK)
        free(col_resp_headline.col_length);
    if (col_count > BUFFER_HEADLINE_BLOCK)
        col_resp_headline.col_length = (int*)malloc(col_count * sizeof(int));
    else if (col_count > 0)
        col_resp_headline.col_length = col_resp_headline.block;

    if (col_count > 0) {
        col_resp_headline.col_count = col_count;
        for (i = 0; i < col_count; i++)
            col_resp_headline.col_length[i] = 0;
    }
    else
        col_resp_headline.col_count = 0;

    col_resp_headline.col_index = 0;
}


void fini_resp_buffer()
{
    if (col_resp_list) {
        free(col_resp_list);
        col_resp_list = 0;
    }
    last_col_resp = 0;
}


int add_resp_buffer(int type, int datatype, int length, void* column)
{
    if (!(type == RESPONSE_TITLE ||
          type == RESPONSE_FORMAT ||
          type == RESPONSE_RECEND ||
          type == RESPONSE_DATA))
        return -1;

    int flush_retval;
    if(last_col_resp == BUFFER_PARCELS_LIMIT)
        /* set last_col_resp = 0 in 'flush_resp_buffer'; */
        flush_retval = flush_resp_buffer();
    if (flush_retval == -1)
        return -1;

    column_response *col = &col_resp_list[last_col_resp];

    col->type = type;
    col->datatype = datatype;
    col->length = length;
    if (type == RESPONSE_TITLE) {
        if(title_col_index < 0)
            /* initialize for later use */
            title_col_index = 0;
        if (title_col_index >= col_resp_headline.col_count)
            /* This is impossible */
            title_col_index = 0;
        if (col_resp_headline.col_length[title_col_index] < length)
            col_resp_headline.col_length[title_col_index] = length;
        title_col_index++;

        if (length > BUFFER_PARCEL_BLOCK) {
            col->column.title = (char*)malloc(length);
            memcpy(col->column.title, column, length);
        }
        else if (length > 0) {
            col->column.title = (char*)col->block;
            memcpy(col->column.title, column, length);
        }
    }
    else if (type == RESPONSE_FORMAT) {
        if (length > BUFFER_PARCEL_BLOCK) {
            col->column.format = (char*)malloc(length);
            memcpy(col->column.format, column, length);
        }
        else if (length > 0) {
            col->column.format = (char*)col->block;
            memcpy(col->column.format, column, length);
        }
    }
    else if (type == RESPONSE_DATA) {
        if(data_col_index < 0)
            /* initialize for later use */
            data_col_index = 0;
        if (data_col_index >= col_resp_headline.col_count)
            data_col_index = 0;
        if (col_resp_headline.col_length[data_col_index] < length)
            col_resp_headline.col_length[data_col_index] = length;
        data_col_index++;

        data_in_resp_list = 1;

        if (length > BUFFER_PARCEL_BLOCK) {
            col->column.data = (char*)malloc(length);
            if (col->column.data)
                memcpy(col->column.data, column, length);
            else {
                if(DEBUG_ON & 0xF0)
                    fprintf(stderr, "Cannot alloc enough memory with length %d.\n", length);
                col->column.data = (char*)col->block;
                memcpy(col->column.data, column, BUFFER_PARCEL_BLOCK);
                col->length = BUFFER_PARCEL_BLOCK;
            }
        }
        else if (length > 0) {
            col->column.data = (char*)col->block;
            memcpy(col->column.data, column, length);
        }
    }

    last_col_resp++;

    return 0;
}


static inline void internal_printf (FILE* pf, char ch, int datatype)
{
    if (ch == '\r')
        ch = '\n';
    if (!internal_quiescent_mode && data_in_resp_list)
        fprintf(pf, "%c", ch);

    if (ch == '\n' && datatype == RESPONSE_DATA_SHOWDML)
        internal_row_width = 0;
}


/*
 * Return value: 0 indicates normal, -1 indicates the pipe is closed.
 *               1 indicates there is nothing to show
 *
 * Note that the "Format" parcel does show itself, but the "---" separator line
 *
 * TODO: to provide 'sidetitles on' feature
 */
int flush_resp_buffer()
{
    if(!col_resp_list)
        return 1;

    if(last_col_resp == 0 && col_resp_list[0].type == 0)
        return 1;

    int i, j, last_type = 0;
    int eff_length = 0;
    column_response * ptr = 0;
    FILE* fout = stdout;
    if(internal_pager)
        fout = internal_pager;

    for(i = 0; i < last_col_resp; i++) {
        ptr = &col_resp_list[i];

        if (ptr->type == RESPONSE_TITLE || ptr->type == RESPONSE_FORMAT || ptr->type == RESPONSE_DATA) {
            eff_length = col_resp_headline.col_length[col_resp_headline.col_index];
            if (++col_resp_headline.col_index >= col_resp_headline.col_count)
                col_resp_headline.col_index = 0;
        }

        if ((last_type == RESPONSE_TITLE || last_type == RESPONSE_FORMAT) && last_type != ptr->type) {
            internal_printf(fout, '\n', 0);

            internal_row_width = 0;
            multi_col_title    = 0;
            multi_col_format   = 0;
        }

        if (ptr->type == RESPONSE_TITLE) {
            if (internal_row_width < output_screen_width && multi_col_title++)
                internal_printf(fout, '|', 0);
            for(j = 0; internal_row_width < output_screen_width && j < ptr->length; internal_row_width++, j++)
                internal_printf(fout, ptr->column.title[j], 0);
            /* pad the title with space */
            for(j = 0; internal_row_width < output_screen_width && j < eff_length - ptr->length; j++, internal_row_width++)
                internal_printf(fout, ' ', 0);

            if(ptr->length > BUFFER_PARCEL_BLOCK)
                free(ptr->column.title);
        }
        else if (ptr->type == RESPONSE_FORMAT) {
            if (internal_row_width < output_screen_width && multi_col_format++)
                internal_printf(fout, '+', 0);
            for(j = 0; internal_row_width < output_screen_width && j < eff_length; internal_row_width++, j++)
                internal_printf(fout, '-', 0);
            if(ptr->length > BUFFER_PARCEL_BLOCK)
                free(ptr->column.format);
        }
        else if (ptr->type == RESPONSE_DATA) {
            if (internal_row_width < output_screen_width && multi_col_data++)
                internal_printf(fout, '|', 0);
            for(j = 0; internal_row_width < output_screen_width && j < ptr->length; internal_row_width++, j++)
                internal_printf(fout, ptr->column.data[j], ptr->datatype);
            /* pad the data with space */
            for(j = 0; internal_row_width < output_screen_width && j < eff_length - ptr->length; j++, internal_row_width++)
                internal_printf(fout, ' ', 0);

            if(ptr->length > BUFFER_PARCEL_BLOCK)
                free(ptr->column.data);
        }
        else if (ptr->type == RESPONSE_RECEND) {
            internal_printf(fout, '\n', 0);

            internal_row_width = 0;
            multi_col_data = 0;
        }

        last_type = ptr->type;

        ptr->type = 0;
        ptr->length = 0;
    }

    if (last_col_resp < BUFFER_PARCELS_LIMIT) {
        internal_printf(fout, '\n', 0);

        internal_row_width = 0;
        multi_col_title    = 0;
        multi_col_format   = 0;
    }

    last_col_resp     = 0;
    data_in_resp_list = 0;

    /* they're managed internally by 'add_resp_buffer'
    title_col_index = -1;
    data_col_index  = -1;
    */

    return 0;
}

