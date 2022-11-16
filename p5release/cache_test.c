#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "cache.h"
#include "print_helpers.h"


void testMake(){
    cache_t *test1 = make_cache(1024, 4, 2, NONE, false);

    printf("Expected n_set: 128, actual: %d\n", test1->n_set);
    printf("Expected n_cache_line: 256, actual: %d\n", test1->n_cache_line);
    printf("Expected n_offset_bit: 2, actual: %d\n", test1->n_offset_bit);
    printf("Expected n_index_bit: 7, actual: %d\n", test1->n_index_bit);
    printf("Expected n_tag_bit: 23, actual: %d\n", test1->n_tag_bit);
}

void testTag(){
    cache_t *test1 = make_cache(1024, 4, 2, NONE, false);
    unsigned long addr = 0xfffffe00;
    printf("Expected Tag: 0x0007fffff, actual: %lx\n", get_cache_tag(test1, addr));
}

void testIndex(){
    cache_t *test1 = make_cache(1024, 4, 2, NONE, false);
    unsigned long addr = 0x000001fc;
    printf("Expected Tag: 0x0000007f, actual: %lx\n", get_cache_index(test1, addr));

}
void testOffSet(){
    cache_t *test1 = make_cache(1024, 4, 2, NONE, false);
    unsigned long addr = 0b0111;
    printf("Expected Tag: 0x4, actual: %ld\n", get_cache_block_addr(test1, addr));
}

void testAccessBasic(){
    cache_t *test1 = make_cache(1024, 4, 2, NONE, false);

    // 0b1 1111111 00
    // 0b11 1111111 00
    printf("Expected: false, Actual: %d\n", access_cache(test1,  0b1111111100, LOAD));
    printf("Expected: false, Actual: %d\n", access_cache(test1, 0b11111111100, LOAD));
    printf("Expected: true, Actual: %d\n", access_cache(test1,   0b1111111100, LOAD));
    printf("Expected: true, Actual: %d\n", access_cache(test1,  0b11111111100, LOAD));

}
int main(){
    // testMake();
    // testTag();
    // testIndex();
    // testOffSet();
    testAccessBasic();
    return 0;
}