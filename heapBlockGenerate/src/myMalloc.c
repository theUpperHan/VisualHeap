#include <errno.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "myMalloc.h"
#include "printing.h"

/* Due to the way assert() prints error messges we use out own assert function
 * for deteminism when testing assertions
 */
#ifdef TEST_ASSERT
  inline static void assert(int e) {
    if (!e) {
      const char * msg = "Assertion Failed!\n";
      write(2, msg, strlen(msg));
      exit(1);
    }
  }
#else
  #include <assert.h>
#endif

/*
 * Mutex to ensure thread safety for the my_freelist
 */
static pthread_mutex_t mutex;

/*
 * Array of sentinel nodes for the my_freelists
 */
header freelistSentinels[N_LISTS];

/*
 * Pointer to the second fencepost in the most recently allocated chunk from
 * the OS. Used for coalescing chunks
 */
header * lastFencePost;

/*
 * Pointer to maintian the base of the heap to allow printing based on the
 * distance from the base of the heap
 */ 
void * base;

/*
 * List of chunks allocated by  the OS for printing boundary tags
 */
header * osChunkList [MAX_OS_CHUNKS];
size_t numOsChunks = 0;

/*
 * direct the compiler to run the init function before running main
 * this allows initialization of required globals
 */
static void init (void) __attribute__ ((constructor));

// Helper functions for manipulating pointers to headers
static inline header * get_header_from_offset(void * ptr, ptrdiff_t off);
static inline header * get_left_header(header * h);
static inline header * ptr_to_header(void * p);

// Helper functions for allocating more memory from the OS
static inline void initialize_fencepost(header * fp, size_t left_size);
static inline void insert_os_chunk(header * hdr);
static inline void insert_fenceposts(void * raw_mem, size_t size);
static header * allocate_chunk(size_t size);

// Helper functions for my_freeing a block
static inline void deallocate_object(void * p);

// Helper functions for allocating a block
static inline header * allocate_object(size_t raw_size);

// Helper functions for verifying that the data structures are structurally 
// valid
static inline header * detect_cycles();
static inline header * verify_pointers();
static inline bool verify_my_freelist();
static inline header * verify_chunk(header * chunk);
static inline bool verify_tags();

static void init();

static bool isMallocInitialized;

/**
 * @brief Helper function to retrieve a header pointer from a pointer and an 
 *        offset
 *
 * @param ptr base pointer
 * @param off number of bytes from base pointer where header is located
 *
 * @return a pointer to a header offset bytes from pointer
 */
static inline header * get_header_from_offset(void * ptr, ptrdiff_t off) {
	return (header *)((char *) ptr + off);
}

/**
 * @brief Helper function to get the header to the right of a given header
 *
 * @param h original header
 *
 * @return header to the right of h
 */
header * get_right_header(header * h) {
	return get_header_from_offset(h, get_size(h));
}

/**
 * @brief Helper function to get the header to the left of a given header
 *
 * @param h original header
 *
 * @return header to the right of h
 */
inline static header * get_left_header(header * h) {
  return get_header_from_offset(h, -h->left_size);
}

/**
 * @brief Fenceposts are marked as always allocated and may need to have
 * a left object size to ensure coalescing happens properly
 *
 * @param fp a pointer to the header being used as a fencepost
 * @param left_size the size of the object to the left of the fencepost
 */
inline static void initialize_fencepost(header * fp, size_t left_size) {
	set_state(fp,FENCEPOST);
	set_size(fp, ALLOC_HEADER_SIZE);
	fp->left_size = left_size;
}

/**
 * @brief Helper function to maintain list of chunks from the OS for debugging
 *
 * @param hdr the first fencepost in the chunk allocated by the OS
 */
inline static void insert_os_chunk(header * hdr) {
  if (numOsChunks < MAX_OS_CHUNKS) {
    osChunkList[numOsChunks++] = hdr;
  }
}

/**
 * @brief given a chunk of memory insert fenceposts at the left and 
 * right boundaries of the block to prevent coalescing outside of the
 * block
 *
 * @param raw_mem a void pointer to the memory chunk to initialize
 * @param size the size of the allocated chunk
 */
inline static void insert_fenceposts(void * raw_mem, size_t size) {
  // Convert to char * before performing operations
  char * mem = (char *) raw_mem;

  // Insert a fencepost at the left edge of the block
  header * leftFencePost = (header *) mem;
  initialize_fencepost(leftFencePost, ALLOC_HEADER_SIZE);

  // Insert a fencepost at the right edge of the block
  header * rightFencePost = get_header_from_offset(mem, size - ALLOC_HEADER_SIZE);
  initialize_fencepost(rightFencePost, size - 2 * ALLOC_HEADER_SIZE);
}

/**
 * @brief Allocate another chunk from the OS and prepare to insert it
 * into the free list
 *
 * @param size The size to allocate from the OS
 *
 * @return A pointer to the allocable block in the chunk (just after the 
 * first fencepost)
 */
static header * allocate_chunk(size_t size) {
  void * mem = sbrk(size);
  
  insert_fenceposts(mem, size);
  header * hdr = (header *) ((char *)mem + ALLOC_HEADER_SIZE);
  set_state(hdr, UNALLOCATED);
  set_size(hdr, size - 2 * ALLOC_HEADER_SIZE);
  hdr->left_size = ALLOC_HEADER_SIZE;
  return hdr;
}


/**
 *  Helper Functions
 */
static int get_sentinel_index(size_t raw_size) {
  int sentinel_index = (raw_size / MIN_ALLOCATION) - 1;
  sentinel_index = sentinel_index >= N_LISTS - 1 ? N_LISTS - 1 : sentinel_index;
  return sentinel_index;
}

static void put_into_freelist(header * block) {
  int block_sentinel_index = get_sentinel_index(get_size(block) - ALLOC_HEADER_SIZE);
  header * sentinel = &freelistSentinels[block_sentinel_index];
  header * first_block = sentinel->next;
  sentinel->next = block;
  first_block->prev = block;
  block->prev = sentinel;
  block->next = first_block;
}

static bool isValidSize(size_t size) {
  if (size % 8 != 0)
    return false;
  if (size < 0)
    return false;
  if (size > MAX_OS_CHUNKS * ARENA_SIZE)
    return false;
  return true;
}


/**
 * @brief Helper allocate an object given a raw request size from the user
 *
 * @param raw_size number of bytes the user needs
 *
 * @return A block satisfying the user's request
 */
static inline header * allocate_object(size_t raw_size) {
  // An allocation of 0 bytes should return the NULL pointer for determinism
  if (raw_size <= 0) {
    return NULL;
  }

  // All requests from the user are rounded up to the nearest multiple of 8 bytes
  if (raw_size % 8 != 0) {
    raw_size = ((raw_size / MIN_ALLOCATION) + 1) * MIN_ALLOCATION;
  }
  raw_size = raw_size > 2 * sizeof(header *) ? raw_size : 2 * sizeof(header *);

  // Find if there are free block fits the required raw size
  header * allocate_block = NULL;
  // for (int sentinel_index = get_sentinel_index(raw_size); sentinel_index <= N_LISTS - 1; sentinel_index++) {
  for (int sentinel_index = get_sentinel_index(raw_size); sentinel_index <= N_LISTS - 1; sentinel_index++) {
    header * sentinel = &freelistSentinels[sentinel_index];
    header * my_free_block = sentinel->next;
    // loop below is for the N_LISTS-1 free list
    while (my_free_block != sentinel && allocate_block == NULL) {
      if (get_size(my_free_block) - ALLOC_HEADER_SIZE >= raw_size) {
        allocate_block = my_free_block;
        break;
      }
      my_free_block = my_free_block->next;
    }
  }

  // Do not find enough memory space (free block) in current chunk
  if (allocate_block == NULL) {
    size_t new_block_size = 0;
    while(new_block_size < raw_size && numOsChunks < MAX_OS_CHUNKS) {
      // Get new chunk if possible
      header * new_chunk = allocate_chunk(ARENA_SIZE);
      if (new_chunk == NULL) {
        return NULL;
      }
      // Check if chunk need coalescing
      header * new_chunk_left_fencepost = get_left_header(new_chunk);
      header * new_chunk_right_fencepost = get_right_header(new_chunk);
      header * last_block = get_left_header(lastFencePost);
      if (lastFencePost == get_left_header(get_left_header(new_chunk))) {
        // Coalescing with left chunk
        lastFencePost = new_chunk_right_fencepost;
        if (get_state(last_block) == UNALLOCATED) {
          // Left block is unallocated
          int last_block_sentinel_index = get_sentinel_index(get_size(last_block) - ALLOC_HEADER_SIZE);
          set_size(last_block, get_size(last_block) + ARENA_SIZE);
          new_chunk_right_fencepost->left_size = get_size(last_block);
          header * new_block = last_block;
          int new_block_sentinel_index = get_sentinel_index(get_size(new_block) - ALLOC_HEADER_SIZE);
          // Put into my_freelist
          if (last_block_sentinel_index != new_block_sentinel_index) {
            // Unlink from original list if not in last
            header * prev_block_of_new = new_block->prev;
            header * next_block_of_new = new_block->next;
            new_block->prev->next = next_block_of_new;
            new_block->next->prev = prev_block_of_new;
            // Link to new list
            put_into_freelist(new_block);
          }
          new_block_size = get_size(new_block);
          if (new_block_size >= raw_size && numOsChunks < MAX_OS_CHUNKS) {
            allocate_block = new_block;
            break;
          }
        }
        // Left block is allocated
        else if (get_state(last_block) != UNALLOCATED) {
          new_chunk_right_fencepost->left_size = ARENA_SIZE;
          header * new_block = get_right_header(last_block);
          set_size_and_state(new_block, ARENA_SIZE, UNALLOCATED);
          // put into free list
          put_into_freelist(new_block);
          new_block_size = get_size(new_block);
          if (new_block_size >= raw_size && numOsChunks < MAX_OS_CHUNKS) {
            allocate_block = new_block;
            break;
          }
        }
      }
      // Do not coalesing chunk
      else if (lastFencePost != get_left_header(new_chunk_left_fencepost)) {
        insert_os_chunk(new_chunk_left_fencepost);
        new_block_size = ARENA_SIZE - ALLOC_HEADER_SIZE * 2;
        header * new_block = new_chunk;
        put_into_freelist(new_block);
        new_block_size = get_size(new_block);
        if (new_block_size >= raw_size && numOsChunks < MAX_OS_CHUNKS) {
          allocate_block = new_block;
          break;
        }
      }
    }
  }
  // Check if get enough space
  if (allocate_block == NULL) {
    return NULL;
  }
  // If the block is exactly the request size the block is simply removed from the free list
  if (get_size(allocate_block) - ALLOC_HEADER_SIZE == raw_size) {
    header * prev_block = allocate_block->prev;
    header * next_block = allocate_block->next;
    prev_block->next = next_block;
    next_block->prev = prev_block;
    set_state(allocate_block, ALLOCATED);
    return get_header_from_offset(allocate_block, ALLOC_HEADER_SIZE);
  }
  // If the block is larger than the request size,
  // but the remainder is too small to be allocated on its own
  // the extra memory is included in the memory allocated to the user
  // and the full block is still allocated just as if it had been exactly the right size.
  else if (get_size(allocate_block) - raw_size < ALLOC_HEADER_SIZE) {
    header * prev_block = allocate_block->prev;
    header * next_block = allocate_block->next;
    prev_block->next = next_block;
    next_block->prev = prev_block;
    set_state(allocate_block, ALLOCATED);
    return get_header_from_offset(allocate_block, ALLOC_HEADER_SIZE);
  }
  // If the block is larger than the request size
  // and the remainder is large enough to be allocated on its own
  // the block is split into two smaller blocks
  // We could allocate either of the blocks to the user
  // but for determinism the user is allocated the block which is higher in memory
  // (i.e. the rightmost block)
  else if (get_size(allocate_block) - raw_size >= ALLOC_HEADER_SIZE) {
    header * split_block = allocate_block;
    int split_block_original_sentinel_index = get_sentinel_index(get_size(split_block) - ALLOC_HEADER_SIZE);
    set_size(split_block, get_size(allocate_block) - raw_size - ALLOC_HEADER_SIZE);
    allocate_block = get_right_header(split_block);
    allocate_block->left_size = get_size(split_block);
    set_size_and_state(allocate_block, raw_size + ALLOC_HEADER_SIZE, ALLOCATED);
    header * right_block = get_right_header(allocate_block);
    right_block->left_size = get_size(allocate_block);
    int split_block_current_sentinel_index = get_sentinel_index(get_size(split_block) - ALLOC_HEADER_SIZE);
    // Check if split_block should relink in the my_freelist
    // Put into my_freelist
    if (split_block_original_sentinel_index != split_block_current_sentinel_index) {
      // Unlink from original list if not in last
      header * prev_block = split_block->prev;
      header * next_block = split_block->next;
      prev_block->next = next_block;
      next_block->prev = prev_block;
      // Link to new list
      put_into_freelist(split_block);
    }
    return get_header_from_offset(allocate_block, ALLOC_HEADER_SIZE);
  }

  // (void) raw_size;
  // assert(false);
  // exit(1);
}

/**
 * @brief Helper to get the header from a pointer allocated with malloc
 *
 * @param p pointer to the data region of the block
 *
 * @return A pointer to the header of the block
 */
static inline header * ptr_to_header(void * p) {
  return (header *)((char *) p - ALLOC_HEADER_SIZE); //sizeof(header));
}

/**
 * @brief Helper to manage deallocation of a pointer returned by the user
 *
 * @param p The pointer returned to the user by a call to malloc
 */
static inline void deallocate_object(void * p) {
  // my_freeing a NULL pointer is a no-op (don’t do anything)
  if (p == NULL) {
    return;
  }
  header * deallocate_block = get_header_from_offset((header *)p, 0 - ALLOC_HEADER_SIZE);
  if (get_state(deallocate_block) != ALLOCATED) {
    const char * msg = "Double Free Detected\n";
    write(2, msg, strlen(msg));  // Get from expected test result
    assert(false);  // Get from expected test result
    exit(01);  // Get from expected test result
  }
  if (!isValidSize(get_size(deallocate_block)) || !isValidSize(deallocate_block->left_size)) {
    return;
  }
  header * left_block = get_left_header(deallocate_block);
  header * right_block = get_right_header(deallocate_block);
  // Neither the right nor the left blocks are unallocated
  // In this case, simply insert the block into the appropriate free list
  if (get_state(left_block) != UNALLOCATED && get_state(right_block) != UNALLOCATED) {
    set_state(deallocate_block, UNALLOCATED);
    put_into_freelist(deallocate_block);
  }
  // Only the right block is unallocated
  // Then coalesce the current and right blocks together
  // The newly coalesced block should remain where the right block was in the free list
  else if (get_state(left_block) != UNALLOCATED && get_state(right_block) == UNALLOCATED) {
    set_size_and_state(deallocate_block, get_size(deallocate_block) + get_size(right_block), UNALLOCATED);
    header * right_block_right = get_right_header(right_block);
    right_block_right->left_size = get_size(deallocate_block);
    int right_block_sentinel_index = get_sentinel_index(get_size(right_block) - ALLOC_HEADER_SIZE);
    int deallocate_block_sentinel_index = get_sentinel_index(get_size(deallocate_block) - ALLOC_HEADER_SIZE);
    // Put into my_freelist
    if (right_block_sentinel_index != deallocate_block_sentinel_index) {
      // Unlink from original list if not in last
      header * prev_block = right_block->prev;
      header * next_block = right_block->next;
      prev_block->next = next_block;
      next_block->prev = prev_block;
      // Link to new list
      put_into_freelist(deallocate_block);
    }
    else if (right_block_sentinel_index == deallocate_block_sentinel_index) {
      header * prev_block = right_block->prev;
      header * next_block = right_block->next;
      prev_block->next = deallocate_block;
      next_block->prev = deallocate_block;
      deallocate_block->prev = prev_block;
      deallocate_block->next = next_block;
    }
  }
  // Only the left block is unallocated
  // Then coalesce the current and left blocks and the newly coalesced block should remain where the left block was in the free list
  else if (get_state(left_block) == UNALLOCATED && get_state(right_block) != UNALLOCATED) {
    set_state(deallocate_block, UNALLOCATED);  // Double free checker
    int left_block_original_sentinel_index = get_sentinel_index(get_size(left_block) - ALLOC_HEADER_SIZE);
    set_size_and_state(left_block, get_size(left_block) + get_size(deallocate_block), UNALLOCATED);
    right_block->left_size = get_size(left_block);
    int left_block_sentinel_index = get_sentinel_index(get_size(left_block) - ALLOC_HEADER_SIZE);
    // Put into freelist
    if (left_block_original_sentinel_index != left_block_sentinel_index) {
      // Unlink from original list if not in last
      header * prev_block = left_block->prev;
      header * next_block = left_block->next;
      prev_block->next = next_block;
      next_block->prev = prev_block;
      // Link to new list
      put_into_freelist(left_block);
    }
  }
  // Both the right and left blocks are unallocated
  // We must coalesce with both neighbors
  // In this case the coalesced block should remain where the left block (lower in memory) was in the free list.
  else if (get_state(left_block) == UNALLOCATED && get_state(right_block) == UNALLOCATED) {
    set_state(deallocate_block, UNALLOCATED);  // Double free checker
    int left_block_original_sentinel_index = get_sentinel_index(get_size(left_block) - ALLOC_HEADER_SIZE);
    set_size_and_state(left_block, get_size(left_block) + get_size(deallocate_block) + get_size(right_block), UNALLOCATED);
    header * right_block_right = get_right_header(right_block);
    right_block_right->left_size = get_size(left_block);
    int left_block_sentinel_index = get_sentinel_index(get_size(left_block) - ALLOC_HEADER_SIZE);
    // Put into my_freelist
    header * right_prev_block = right_block->prev;
    header * right_next_block = right_block->next;
    right_prev_block->next = right_next_block;
    right_next_block->prev = right_prev_block;
    if (left_block_original_sentinel_index != left_block_sentinel_index) {
      // Unlink from original list if not in last
      header * left_prev_block = left_block->prev;
      header * left_next_block = left_block->next;
      left_prev_block->next = left_next_block;
      left_next_block->prev = left_prev_block;
      // Link to new list
      put_into_freelist(left_block);
    }
  }

  // (void) p;
  // assert(false);
  // exit(1);
}
/**
 * @brief Helper to detect cycles in the free list
 * https://en.wikipedia.org/wiki/Cycle_detection#Floyd's_Tortoise_and_Hare
 *
 * @return One of the nodes in the cycle or NULL if no cycle is present
 */
static inline header * detect_cycles() {
  for (int i = 0; i < N_LISTS; i++) {
    header * freelist = &freelistSentinels[i];
    for (header * slow = freelist->next, * fast = freelist->next->next;
         fast != freelist;
         slow = slow->next, fast = fast->next->next) {
      if (slow == fast) {
        return slow;
      }
    }
  }
  return NULL;
}

/**
 * @brief Helper to verify that there are no unlinked previous or next pointers
 *        in the free list
 *
 * @return A node whose previous and next pointers are incorrect or NULL if no
 *         such node exists
 */
static inline header * verify_pointers() {
  for (int i = 0; i < N_LISTS; i++) {
    header * freelist = &freelistSentinels[i];
    for (header * cur = freelist->next; cur != freelist; cur = cur->next) {
      if (cur->next->prev != cur || cur->prev->next != cur) {
        return cur;
      }
    }
  }
  return NULL;
}

/**
 * @brief Verify the structure of the free list is correct by checkin for
 *        cycles and misdirected pointers
 *
 * @return true if the list is valid
 */
static inline bool verify_freelist() {
  header * cycle = detect_cycles();
  if (cycle != NULL) {
    fprintf(stderr, "Cycle Detected\n");
    print_sublist(print_object, cycle->next, cycle);
    return false;
  }

  header * invalid = verify_pointers();
  if (invalid != NULL) {
    fprintf(stderr, "Invalid pointers\n");
    print_object(invalid);
    return false;
  }

  return true;
}

/**
 * @brief Helper to verify that the sizes in a chunk from the OS are correct
 *        and that allocated node's canary values are correct
 *
 * @param chunk AREA_SIZE chunk allocated from the OS
 *
 * @return a pointer to an invalid header or NULL if all header's are valid
 */
static inline header * verify_chunk(header * chunk) {
 if (get_state(chunk) != FENCEPOST) {
  fprintf(stderr, "Invalid fencepost\n");
  print_object(chunk);
  return chunk;
 }

 for (; get_state(chunk) != FENCEPOST; chunk = get_right_header(chunk)) {
  if (get_size(chunk)  != get_right_header(chunk)->left_size) {
   fprintf(stderr, "Invalid sizes\n");
   print_object(chunk);
   return chunk;
  }
 }

 return NULL;
}

/**
 * @brief For each chunk allocated by the OS verify that the boundary tags
 *        are consistent
 *
 * @return true if the boundary tags are valid
 */
static inline bool verify_tags() {
  for (size_t i = 0; i < numOsChunks; i++) {
    header * invalid = verify_chunk(osChunkList[i]);
    if (invalid != NULL) {
      return invalid;
    }
  }

  return NULL;
}

/**
 * @brief Initialize mutex lock and prepare an initial chunk of memory for allocation
 */
static void init() {
  // Initialize mutex for thread safety
  pthread_mutex_init(&mutex, NULL);

#ifdef DEBUG
  // Manually set printf buffer so it won't call malloc when debugging the allocator
  setvbuf(stdout, NULL, _IONBF, 0);
#endif // DEBUG

  // Allocate the first chunk from the OS
  header * block = allocate_chunk(ARENA_SIZE);

  header * prevFencePost = get_header_from_offset(block, -ALLOC_HEADER_SIZE);
  insert_os_chunk(prevFencePost);

  lastFencePost = get_header_from_offset(block, get_size(block));

  // Set the base pointer to the beginning of the first fencepost in the first
  // chunk from the OS
  base = ((char *) block) - ALLOC_HEADER_SIZE; //sizeof(header);

  // Initialize freelist sentinels
  for (int i = 0; i < N_LISTS; i++) {
    header * freelist = &freelistSentinels[i];
    freelist->next = freelist;
    freelist->prev = freelist;
  }

  // Insert first chunk into the free list
  header * freelist = &freelistSentinels[N_LISTS - 1];
  freelist->next = block;
  freelist->prev = block;
  block->next = freelist;
  block->prev = freelist;
}

/*
 * External interface
 */
void * my_malloc(size_t size) {
  pthread_mutex_lock(&mutex);
  header * hdr = allocate_object(size);
  pthread_mutex_unlock(&mutex);
  return hdr;
}

void * my_calloc(size_t nmemb, size_t size) {
  return memset(my_malloc(size * nmemb), 0, size * nmemb);
}

void * my_realloc(void * ptr, size_t size) {
  void * mem = my_malloc(size);
  memcpy(mem, ptr, size);
  my_free(ptr);
  return mem;
}

void my_free(void * p) {
  pthread_mutex_lock(&mutex);
  deallocate_object(p);
  pthread_mutex_unlock(&mutex);
}

bool verify() {
  return verify_freelist() && verify_tags();
}