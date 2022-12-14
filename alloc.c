#include "alloc.h"
#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#define MIN_ALLOC_SIZE 4096
#define ALIGN(x) x + x % 8

header_t *FREE_LIST;
header_t *HEAP_HEAD;
unsigned long ACCUM_SIZE;

header_t *more_space(unsigned long size) {
  size = ALIGN(size);
  void *vp;
  if ((vp = sbrk(size)) == (void *)-1) {
    printf("FAIL sbrk\n");
    return NULL;
  }
  ACCUM_SIZE += size;
  return vp;
}

void gc_init() {
  ACCUM_SIZE = 0;
  HEAP_HEAD = more_space(sizeof(header_t)); // dummy
  FREE_LIST = more_space(sizeof(header_t)); // dummy
}

void gc_clean() {
  sbrk(-ACCUM_SIZE);
}

void add_heap(header_t *hp) {
  header_t *tmp = HEAP_HEAD->next;
  HEAP_HEAD->next = hp;
  hp->next = tmp;
}

header_t *create_header_and_add_to_heap() {
  header_t *hp = more_space(sizeof(header_t));
  hp->mark_flag = IN_USED;
  add_heap(hp);
  return hp;
}

header_t *find_header_from_free_list(unsigned long size) {
  header_t* pre = FREE_LIST;
  for(header_t* cur = FREE_LIST->next; cur != NULL;cur = cur->next) {
    if (size == cur->size) {
      pre->next = cur->next;
      return cur; 
    }
    pre = cur;
  }
  return NULL;
}

void *gc_alloc(unsigned long size) {
  size = ALIGN(size); // align to word size

  header_t *free_hp = find_header_from_free_list(size);
  if (free_hp != NULL) {
    free_hp->mark_flag = IN_USED;
    add_heap(free_hp);
    printf("find from free list: %lu\n", free_hp->payload);
    return free_hp->payload;
  } else {
    header_t *hp = create_header_and_add_to_heap();
    hp->mark_flag = IN_USED;
    hp->size = size;
    hp->payload = more_space(size);
    printf("alloc: %lu %lu\n", hp, hp->payload);
    return (hp->payload);
  }
}

unsigned long get_stack_top() {
  unsigned long stack_top;
  asm volatile("movq %%rsp, %0" // move the address of the top of stack
               : "=r"(stack_top));
  return stack_top;
}

unsigned long get_stack_bottom() {
  FILE *statfp;
  unsigned long stack_bottom;
  statfp = fopen("/proc/self/stat", "r");
  assert(statfp != NULL);
  fscanf(statfp,
         "%*d %*s %*c %*d %*d %*d %*d %*d %*u "
         "%*lu %*lu %*lu %*lu %*lu %*lu %*ld %*ld "
         "%*ld %*ld %*ld %*ld %*llu %*lu %*ld "
         "%*lu %*lu %*lu %lu",
         &stack_bottom);
  fclose(statfp);
  return stack_bottom;
}

void print_stack() {
  unsigned long st_bot = get_stack_bottom();
  for (unsigned long st_cur = get_stack_top(); st_cur < st_bot; st_cur = st_cur + 8) {
    printf("%lu\n", *((unsigned long *)st_cur));
  }
}

header_t *is_in_heap(unsigned long p) {
  for (header_t *cur = HEAP_HEAD->next; cur != NULL; cur = cur->next) {
    if ((unsigned long)cur->payload <= p &&
        p <= ((unsigned long)cur->payload + cur->size - 8)) {
      return cur;
    }
  }
  return NULL;
}


void foreach_headers(void (*f)(header_t*), header_t* begin) {
  for (header_t *cur = begin; cur != NULL; cur = cur->next)
    f(cur);
}

void foreach_heap(void (*f)(header_t*)) {
  foreach_headers(f, HEAP_HEAD->next); // except dummy
}

void foreach_free_list(void (*f)(header_t*)) {
  foreach_headers(f, FREE_LIST->next); // except dummy
}

void unmark_header(header_t* cur) {
  cur->mark_flag = NOT_USED;
}

void unmark_heap() {
  foreach_heap(unmark_header);
}

void print_header(header_t* cur) {
    printf("%d %lu %d\n", cur->size, (unsigned long)cur->payload,
           cur->mark_flag);
}

void print_heap() {
  printf("HEAP: \n");
  foreach_heap(print_header);
}

void print_free_list() {
  printf("FREE_LIST: \n");
  foreach_free_list(print_header);
}


void add_to_free_list(header_t* hp) {
  header_t *tmp = FREE_LIST->next;
  FREE_LIST->next = hp;
  hp->next = tmp;
}

void gc_mark() {
  unmark_heap(); // set all to NOT_USED
  unsigned long st_bot = get_stack_bottom();
  for (unsigned long st_cur = get_stack_top(); st_cur < st_bot; st_cur = st_cur + 8) {
    header_t *hp = is_in_heap(*((unsigned long *)st_cur));
    if (hp != NULL) hp->mark_flag = IN_USED;
  }
}

void gc_sweep() {
  printf("SWEEP BEGIN:\n");
  print_free_list();
  print_heap();
  printf("\n");
  header_t *pre = HEAP_HEAD;
  for (header_t *cur = HEAP_HEAD->next; cur != NULL;) {
    if (cur->mark_flag == NOT_USED) {
      pre->next = cur->next;
      add_to_free_list(cur); // has side effect on cur
      cur = pre->next;
    } else {
      pre = cur;
      cur = cur->next;
    }
  }
  printf("SWEEP END:\n");
  print_free_list();
  print_heap();
  printf("\n");
}

