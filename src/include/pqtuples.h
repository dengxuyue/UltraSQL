#ifndef __PQTUPLES_H__
#define __PQTUPLES_H__

typedef struct pq_tuple_t {
    int   length;
    char* data;

    struct pq_tuple_t* next;
} pq_tuple;

void set_pq_sidetitles_impl(int on_off);

#endif
