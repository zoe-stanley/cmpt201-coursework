#define _DEFAULT_SOURCE
#define _ISOC99_SOURCE
// some of these defines were added based on the reference code, like BUF_SIZE
// and DEFAULT_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUF_SIZE 64

struct header {
  uint64_t size;
  struct header *next;
};

void handle_error(const char *msg) { perror(msg); }

void print_out(char *format, void *data, size_t data_size) {
  char buf[BUF_SIZE];
  ssize_t len = snprintf(buf, BUF_SIZE, format,
                         data_size == sizeof(uint64_t) ? *(uint64_t *)data
                                                       : *(void **)data);

  if (len < 0) {
    handle_error("snprintf");
  }
  write(STDOUT_FILENO, buf, len);
}

// example usage
//  int waldo = 42
//  print_out("where is waldo? %p\n", &waldo, sizeof(&waldo));

int main() {
  void *oldbreak = sbrk(0); // get the current breakpoint
  // initialize memory block headers
  if (sbrk(256) == (void *)-1) {
    handle_error("sbrk() error");
  }

  struct header *first_block = (struct header *)oldbreak;
  struct header *second_block =
      (struct header *)((char *)oldbreak + 128); // set it to point to the
                                                 // end of the first block
  first_block->size = 128;
  first_block->next = NULL;

  second_block->size = 128;
  second_block->next = first_block;

  memset((char *)first_block + sizeof(struct header), 0,
         128 - sizeof(struct header)); // set all zero values

  memset((char *)second_block + sizeof(struct header), 1,
         128 - sizeof(struct header));

  print_out("first block: %p\n", &first_block, sizeof(first_block));
  print_out("second block: %p\n", &second_block, sizeof(second_block));

  print_out("first block size: %lu\n", &first_block->size,
            sizeof(first_block->size));
  print_out("first block next: %p\n", &first_block->next,
            sizeof(first_block->next));
  print_out("second block size: %lu\n", &second_block->size,
            sizeof(second_block->size));

  print_out("second block next: %p\n", &second_block->next,
            sizeof(second_block->next));

  uint8_t *first = (uint8_t *)((char *)first_block + sizeof(struct header));
  size_t n = 128 - sizeof(struct header);

  for (size_t i = 0; i < n; i++) {
    uint64_t copy = first[i];
    print_out("%lu\n", &copy, sizeof(copy));
  }

  uint8_t *second = (uint8_t *)((char *)second_block + sizeof(struct header));
  for (size_t i = 0; i < n; i++) {
    uint64_t copy = second[i];
    print_out("%lu\n", &copy, sizeof(copy));
  }

  return 0;
}
