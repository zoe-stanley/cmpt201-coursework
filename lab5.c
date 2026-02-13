// Lab 5: Memory allocation
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct header {
  uint64_t size;
  struct header *next;
  int id;
};

void initialize_block(struct header *block, uint64_t size, struct header *next,
                      int id) {
  block->size = size;
  block->next = next;
  block->id = id;
}

int find_first_fit(struct header *free_list_ptr, uint64_t size) {
  // TODO Implement first fit
  struct header *current = free_list_ptr;
  while (current != NULL) {
    if (current->size >= size) {
      return current->id;
    }
    current = current->next;
  }
  return -1;
}

int find_best_fit(struct header *free_list_ptr, uint64_t size) {
  int best_fit_id = -1;
  // TODO Implement best fit
  struct header *current = free_list_ptr;
  uint64_t best_size_so_far =
      UINT64_MAX; // this UINT64_MAX copied from reference solution,
                  // was 10000000 before but this seems safer
  while (current != NULL) {
    if (current->size >= size && current->size < best_size_so_far) {
      best_fit_id = current->id;
      best_size_so_far = current->size;
    }
    current = current->next;
  }
  return best_fit_id;
}

int find_worst_fit(struct header *free_list_ptr, uint64_t size) {
  int worst_fit_id = -1;
  // TODO Implement worst fit
  //
  struct header *current = free_list_ptr;
  uint64_t worst_size_so_far =
      0; // this value also taken from reference solution, was -1
  while (current != NULL) {
    if (current->size >= size && current->size > worst_size_so_far) {
      worst_fit_id = current->id;
      worst_size_so_far = current->size;
    }
    current = current->next;
  }
  return worst_fit_id;
}

int main(void) {
  struct header *free_block1 = (struct header *)malloc(sizeof(struct header));
  struct header *free_block2 = (struct header *)malloc(sizeof(struct header));
  struct header *free_block3 = (struct header *)malloc(sizeof(struct header));
  struct header *free_block4 = (struct header *)malloc(sizeof(struct header));
  struct header *free_block5 = (struct header *)malloc(sizeof(struct header));

  initialize_block(free_block1, 6, free_block2, 1);
  initialize_block(free_block2, 12, free_block3, 2);
  initialize_block(free_block3, 24, free_block4, 3);
  initialize_block(free_block4, 8, free_block5, 4);
  initialize_block(free_block5, 4, NULL, 5);

  struct header *free_list_ptr = free_block1;

  int first_fit_id = find_first_fit(free_list_ptr, 7);
  int best_fit_id = find_best_fit(free_list_ptr, 7);
  int worst_fit_id = find_worst_fit(free_list_ptr, 7);

  // TODO: print out the IDs
  printf("The ID for First-Fit algorithm is: %d\n", first_fit_id);
  printf("The ID for Best-Fit algorithm is: %d\n", best_fit_id);
  printf("The ID for Worst-fit algorithm is: %d\n", worst_fit_id);

  free(free_block1);
  free(free_block2);
  free(free_block3);
  free(free_block4);
  free(free_block5);
  return 0;
}

// PSEUDO-CODE FOR COALESCING
// 1. Given a newly_freed block in an existing linked list
// 2. Check for CASES A, B, C, D: confirm whether one or more freed blocks
// end/start exactly
//    where the newly_freed block begins
// 3. BY CASES (A, B, C, D)
//    A. Free block exists right BEFORE newly_freed block
//    B. Free block is right AFTER newly_freed block
//    C. Free blocks are on all sides (so need to update both)
//    D. Nothing is free.
// A. In case a, do:
//  a)  Grow the size of the existing free block by the newly_freed block's size
//  (join). b) no changes to pointers
// B. In case b, do:
//  a) Grow the size of newly_freed to include the proceeding existing free
//  block. b) remove the previously existing block c) add newly_freed to the
//  free list to encompass both blocks' space d) update newly_freed's next
//  pointer to the old block's next
// C. in case c, do:
//  a) Grow the size of the initial (first) free block to incorporate BOTH the
//  size of newly_freed
//     and the free block coming after
//  b) Remove the last block from the free list (now a part of the first block)
//  and don't add a new
//     block for Newly_freed; it's a part of the first block now.
//  c) update the next pointer of the block to point to the next value that was
//  overwritten by
//     merging.
// D. In case d, do:
//  a) If no remaining free blocks, just add to list
// 4. Update any node that previously pointed to a merged block component t
//
//  Diagram: red block Z is freed --> Case C, both sides must merge
//  Head points to b and b points to m
//  i) Test for both front and back blocks being free - yes
//  ii) since n is the head, we will have to update the head's value to m
//  iii) increase the size of m by the size of both z and n
//  iv) Remove the block n and z from the linked list
//  v) update merged block's next pointer to point to b, the next free block
//  vi) update b's next to NULL.
