#ifndef ALLOC_H
#define ALLOC_H

#define NOT_USED 0
#define IN_USED 1

typedef struct header {
  unsigned int mark_flag;
  unsigned int size;
  struct header *next;
  void *payload;
} header_t;

extern header_t *FREE_LIST;
extern header_t *FREE_LIST_TAIL;
extern header_t *HEAP_HEAD;
extern header_t *HEAP_TAIL;
extern unsigned long ACCUM_SIZE;

void gc_init();
void *gc_alloc(unsigned long size);
void gc_clean();
header_t *more_space(unsigned long size);
unsigned long get_stack_top();
unsigned long get_stack_bottom();
void gc_mark();
void gc_sweep();
header_t* is_in_heap(unsigned long );
void print_heap();
void print_free_list();
void print_stack();

#endif
