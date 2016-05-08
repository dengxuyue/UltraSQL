/*
 * summary: display module for output from PostgreSQL
 *
 * Status:
 *    'sidetitles on' feature (WIP)
 */

#include "config.h"
#include "pqtuples.h"
#include "pager.h"
#include <string.h>
#include <stdio.h>

int max_title_attr_length = 0;
pq_tuple *title = 0;
pq_tuple *title_ending = 0;
pq_tuple *tuple = 0;
pq_tuple *tuple_ending = 0;

static int sidetitles_pqlib = 0;
void set_pq_sidetitles_impl(int on_off)
{
    sidetitles_pqlib = on_off ? 1 : 0;
}


static pq_tuple* add_attribute(int length, char* ptr)
{
    if(length <= 0)
        return 0;
    int len = strlen(ptr);
    if (len <= 0)
        return 0;

    pq_tuple* tup_ptr = (pq_tuple*) malloc (sizeof(pq_tuple));
    tup_ptr->length = MAX(length, len);
    if(len)
        tup_ptr->data = (char*) malloc(sizeof(char) * (len + 1));

    strcpy(tup_ptr->data, ptr);
    tup_ptr->next = 0;

    return tup_ptr;
}


int append_title(int length, char* ptr)
{
    pq_tuple* tp = add_attribute(length, ptr);
    if (tp) {
        if (title) {
            title_ending->next = tp;
            title_ending = tp;
        }
        else {
            title = tp;
            title_ending = tp;
        }
        if (strlen(tp->data) > max_title_attr_length)
            max_title_attr_length = strlen(tp->data);

        return 0;
    }

    return 1;
}


int append_tuple(int length, char* ptr)
{
    pq_tuple* tp = add_attribute(length, ptr);
    if (tp) {
        if (tuple) {
            tuple_ending->next = tp;
            tuple_ending = tp;
        }
        else {
            tuple = tp;
            tuple_ending = tp;
        }
        return 0;
    }

    return 1;
}


static void free_tuple_list(pq_tuple *head)
{
    pq_tuple* tp;
    tp = head;
    while(tp) {
        head = tp->next;
        free(tp->data);
        free(tp);
        tp = head;
    }
}


void flush_title()
{
    if (sidetitles_pqlib)
        return;

    FILE*   fout = stdout;
    int        i = 0, len, k;
    pq_tuple* tp = title;
    while(tp) {
        if(i++) fprintf(fout, "|");

        len = strlen(tp->data);
        fprintf(fout, tp->data);
        if (len < tp->length) {
            for(k = 0; k < tp->length - len; k++)
                fprintf(fout, " ");
        }
        tp = tp->next;
    }
    fprintf(fout, "\n");

    /* separator: ------+------ */
    tp = title;
    i  = 0;
    while(tp) {
        if(i++) fprintf(fout, "+");
        for(k = 0; k < tp->length; k++)
            fprintf(fout, "-");
        tp = tp->next;
    }
    fprintf(fout, "\n");

    /* free "title" list */
    if (!sidetitles_pqlib) {
        free_tuple_list(title);
        title = 0;
    }
}


void flush_tuple ()
{
    FILE* fout = stdout;
    if(internal_pager)
        fout = internal_pager;

    if (sidetitles_pqlib) {
        flush_tuple_pivot(fout);
        return;
    }

    pq_tuple* tp = tuple;
    int k, len, i = 0;
    while(tp) {
        if(i++) fprintf(fout, "|");

        len = strlen(tp->data);
        fprintf(fout, tp->data);
        if (len < tp->length) {
            for(k = 0; k < tp->length - len; k++)
                fprintf(fout, " ");
        }
        tp = tp->next;
    }
    fprintf(fout, "\n");

    /* free "tuple" list */
    free_tuple_list(tuple);
    tuple = 0;
}


void flush_tuple_pivot (FILE* fout)
{
    pq_tuple* up = tuple;
    pq_tuple* ip = title;
    int k;
    int max_tuple_attr_length = 0;
    while(up) {
        fprintf(fout, ip->data);
        for (k = strlen(ip->data); k < max_title_attr_length; k++)
            fprintf(fout, " ");

        fprintf(fout, "|");
        fprintf(fout, up->data);
        if (max_tuple_attr_length < strlen(up->data))
            max_tuple_attr_length = strlen(up->data);


        up = up->next;
        ip = ip->next;
        fprintf(fout, "\n");
    }

    /* free "tuple" list */
    free_tuple_list(tuple);
    tuple = 0;
    for(k = 0; k < MAX(max_title_attr_length + max_tuple_attr_length + 1, 75); k++)
        fprintf(fout, "-");
    fprintf(fout, "\n");
}

void flush_end()
{
    if (sidetitles_pqlib) {
        free_tuple_list(title);
        title = 0;
    }
}
