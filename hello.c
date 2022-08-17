#include <stdio.h>
#include "alloc.h"
#include <assert.h>

void add_something_to_heap();

int main() {
  gc_init();
  int* a = gc_alloc(sizeof(int));
  *a = 3;
  assert(*a == 3);
  add_something_to_heap();
  gc_mark();
  gc_sweep();
  assert(*a == 3);
  return 0;
}

void add_something_to_heap() {
  int *a = gc_alloc(sizeof(int));
  int *b = gc_alloc(sizeof(double));
  gc_mark();
}
