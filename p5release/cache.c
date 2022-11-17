#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "cache.h"
#include "print_helpers.h"

cache_t *make_cache(int capacity, int block_size, int assoc, enum protocol_t protocol, bool lru_on_invalidate_f){
  cache_t *cache = malloc(sizeof(cache_t));
  cache->stats = make_cache_stats();
  
  cache->capacity = capacity;      // in Bytes
  cache->block_size = block_size;  // in Bytes
  cache->assoc = assoc;            // 1, 2, 3... etc.

  // FIX THIS CODE!
  // first, correctly set these 5 variables. THEY ARE ALL WRONG
  // note: you may find math.h's log2 function useful
  cache->n_cache_line = capacity/block_size;
  cache->n_set = capacity/(assoc*block_size);
  cache->n_offset_bit = log2(block_size);
  cache->n_index_bit = log2(capacity/(assoc*block_size));
  cache->n_tag_bit = ADDRESS_SIZE-log2(block_size)-log2(capacity/(assoc*block_size));

  // next create the cache lines and the array of LRU bits
  // - malloc an array with n_rows
  // - for each element in the array, malloc another array with n_col
  // FIX THIS CODE!

  cache->lines = malloc(cache->n_set * sizeof(cache_line_t*));
  for(int i = 0; i< cache->n_set; i++){
    cache->lines[i]=malloc(sizeof(cache_line_t) * assoc);
  }
  cache->lru_way = malloc(cache->n_set * sizeof(int));

  // initializes cache tags to 0, dirty bits to false,
  // state to INVALID, and LRU bits to 0
  // FIX THIS CODE!
  for (int i = 0; i < cache->n_set; i++) {
    for (int j = 0; j < assoc; j++) {
      cache->lines[i][j].dirty_f = false; 
      cache->lines[i][j].tag = 0; 
      cache->lines[i][j].state = INVALID; 
    }
    cache->lru_way[i] = 0;
  }

  cache->protocol = protocol;
  cache->lru_on_invalidate_f = lru_on_invalidate_f;
  
  return cache;
}

/* Given a configured cache, returns the tag portion of the given address.
 *
 * Example: a cache with 4 bits each in tag, index, offset
 * in binary -- get_cache_tag(0b111101010001) returns 0b1111
 * in decimal -- get_cache_tag(3921) returns 15 
 */
unsigned long get_cache_tag(cache_t *cache, unsigned long addr) {
  // FIX THIS CODE!
  return addr >> (ADDRESS_SIZE - cache->n_tag_bit);
}

/* Given a configured cache, returns the index portion of the given address.
 *
 * Example: a cache with 4 bits each in tag, index, offset
 * in binary -- get_cache_index(0b111101010001) returns 0b0101
 * in decimal -- get_cache_index(3921) returns 5
 */
unsigned long get_cache_index(cache_t *cache, unsigned long addr) {
  // FIX THIS CODE!
  addr = addr >> cache->n_offset_bit;
  // printf("addr: %lx\n", addr);
  addr = addr << (cache->n_tag_bit + cache->n_offset_bit);
  unsigned long bitmask = 0;
  bitmask = ~bitmask >> (sizeof(long)*8 - ADDRESS_SIZE);
  addr = addr & bitmask;

  // printf("addr: %lx\n", addr);
  // printf("size of long: %ld\n", sizeof(long));
  // printf("shifted by %d\n", cache->n_tag_bit + cache->n_offset_bit);
  addr = addr >> (cache->n_tag_bit + cache->n_offset_bit);
  
  // printf("addr: %lx\n", addr);

  return addr;
}

/* Given a configured cache, returns the given address with the offset bits zeroed out.
 *
 * Example: a cache with 4 bits each in tag, index, offset
 * in binary -- get_cache_block_addr(0b111101010001) returns 0b111101010000
 * in decimal -- get_cache_block_addr(3921) returns 3920
 */
unsigned long get_cache_block_addr(cache_t *cache, unsigned long addr) {
  addr = addr >> cache->n_offset_bit;
  return addr << cache->n_offset_bit;
}


/**
 * Performs load/store action on the local thread
 * Returns: true if there is a write back, false otherwise 
 * Effect: Increment lru to its supposed location 
 * Effect: log way and log set 
 * Effect: change cache state to VALID 
 * Effect: set the dirty bit appropriately
 * Effect: Update tag as necessary
*/
bool local_load_store(cache_t *cache, unsigned long tag, unsigned long index, enum action_t action, bool hit, int cursor){
  bool writeback_f = false;
  int lru = cache->lru_way[index];


  if(action == LOAD){
    if(!hit){
      // printf("get here 1\n");

      // bring new memory into lru
      if(cache->lines[index][cache->lru_way[index]].dirty_f){
        writeback_f = true; //writeback original dirty data
        // printf("load write back\n");
        cache->lines[index][cache->lru_way[index]].dirty_f = false;
      }

      log_way(lru);
      cache->lines[index][cache->lru_way[index]].tag = tag; // kick original data out, update tag
      // valid data at here
      cache->lines[index][cache->lru_way[index]].state = VALID;

      //increment lru sinced we are kicking original data out. 
      int res = (cache->lru_way[index] +1) % cache->assoc; 
      cache->lru_way[index] = res;
      // printf("read lru %d\n", res);

    }else {
      // if it's a load hit, no data is kicked out, no need to do anything other than logging
      // printf("get here 2\n");
      int res = (cursor + 1) % cache->assoc; // update lru to next
      cache->lru_way[index] = res;
      log_way(cursor);
    }

  }else if(action == STORE){
    if(hit){
      int res = (cursor + 1) % cache->assoc; // update lru to next
      cache->lru_way[index] = res;
      // printf("write hit lru %d\n", res);

      cache->lines[index][cursor].dirty_f = true; // set dirty to the hit line to true
      log_way(cursor);
      
    }else{
      // store miss, writing to lru
      if(cache->lines[index][cache->lru_way[index]].dirty_f){
        // if data at lru is dirty, write back first
        writeback_f = true; 
        // printf("store write back\n");

      }
      
      // log way: 
      log_way(lru);
      // kick out data first
      cache->lines[index][cache->lru_way[index]].tag = tag;
      // set dirty
      cache->lines[index][cache->lru_way[index]].dirty_f = true;
      // valid data at here
      cache->lines[index][cache->lru_way[index]].state = VALID;

      // update lru
      int res = (cache->lru_way[index] + 1) % cache->assoc; 
      cache->lru_way[index] = res;
  
    }
  }

  log_set(index);

  return writeback_f;
}
/* this method takes a cache, an address, and an action
 * it proceses the cache access. functionality in no particular order: 
 *   - look up the address in the cache, determine if hit or miss
 *   - update the LRU_way, cacheTags, state, dirty flags if necessary
 *   - update the cache statistics (call update_stats)
 * return true if there was a hit, false if there was a miss
 * Use the "get" helper functions above. They make your life easier.
 */
bool access_cache(cache_t *cache, unsigned long addr, enum action_t action) {
  
  unsigned long tag = get_cache_tag(cache, addr);
  unsigned long index = get_cache_index(cache, addr);
  
  // printf("get here 2\n");
  // regardless of load or store, if tag match means cache hit
  bool hit = false;
  int cursor = 0; 

  // checking for hits
  while(cursor < cache->assoc){
    
    if(cache->lines[index][cursor].tag == tag && 
        cache->lines[index][cursor].state != INVALID){
      hit = true;
      break;
    }
    cursor++;
  }

  //running load/store cache action on local thread
  bool writeback_f = local_load_store(cache, tag, index, action, hit, cursor);

  //TODO: upgrade_miss_f
  update_stats(cache->stats, hit, writeback_f, false, action);
  return hit;
}
