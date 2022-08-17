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

void gc_init();
void *gc_alloc(unsigned long size);
header_t *more_space(unsigned long size);
unsigned long get_stack_top();
unsigned long get_stack_bottom();
void gc_mark();
void gc_sweep();
header_t* is_in_heap(unsigned long );
void print_heap();

#endif
