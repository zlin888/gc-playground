#include <stdio.h>
#include <assert.h>
#include "alloc.h"

void add_something_to_heap();

int main() {
  gc_init();
  int* a = gc_alloc(sizeof(int));
  *a = 3;
  assert(*a == 3);
  add_something_to_heap();
  gc_mark();
  assert(FREE_LIST->next == NULL);
  assert(HEAP_HEAD->next != NULL);
  assert(HEAP_HEAD->next->next != NULL);
  assert(HEAP_HEAD->next->next->next != NULL);
  gc_sweep();
  assert(FREE_LIST->next != NULL);
  assert(FREE_LIST->next->next != NULL);
  assert(HEAP_HEAD->next != NULL);
  assert(HEAP_HEAD->next->next == NULL);
  assert(*a == 3);
  int* b = gc_alloc(sizeof(int));
  int* c = gc_alloc(sizeof(int));
  *b = 5;
  *c = 6;
  gc_mark();
  gc_sweep();
  assert(FREE_LIST->next == NULL);
  assert(HEAP_HEAD->next != NULL);
  assert(HEAP_HEAD->next->next != NULL);
  assert(HEAP_HEAD->next->next->next != NULL);
  assert(HEAP_HEAD->next->next->next->next == NULL);
  assert(*b == 5);
  assert(*c == 6);
  gc_clean();
  return 0;
}

void add_something_to_heap() {
  int *a = gc_alloc(sizeof(int));
  int *b = gc_alloc(sizeof(double));
}
