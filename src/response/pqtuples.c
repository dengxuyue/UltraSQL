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

pq_tuple *title = 0;
pq_tuple *title_ending = 0;
pq_tuple *tuple = 0;
pq_tuple *tuple_ending = 0;


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
    free_tuple_list(title);
    title = 0;
}


void flush_tuple ()
{
    FILE* fout = stdout;
    if(internal_pager)
        fout = internal_pager;

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

    /* free "title" list */
    free_tuple_list(tuple);
    tuple = 0;
}

void flush_end()
{
}
