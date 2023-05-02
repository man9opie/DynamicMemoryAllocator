/**
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "sfmm.h"
#include "errno.h"

int grow = 0;

//int size_alloc = 0;


void sf_insert_free_block(sf_block *new_block);
sf_block *sf_search_free(sf_block *new_block);

void *sf_malloc(size_t size) {
    // TO BE IMPLEMENTED
    if(size == 0){
        return NULL;
    }

    size_t new_size;
    if(size + 8 < 32){
        new_size = 32;
    }
    else{
        new_size = size +8;
        while(new_size % 8 != 0) {
            new_size++;
        }
    }

    sf_block *ptr = NULL;
    if(grow == 0){// first time run
        grow = 1;
        sf_mem_grow();

        for(int i = 0;i < 10; i++){
            sf_free_list_heads[i].body.links.next = &sf_free_list_heads[i];
            sf_free_list_heads[i].body.links.prev = &sf_free_list_heads[i];
        }

        //quick list
        for(int i = 0; i < 20;i++){
            sf_quick_lists[i].length = 0;
            sf_quick_lists[i].first = NULL;
        }
        sf_block *heap_start = sf_mem_start();
        sf_block *prologue =  heap_start;
        prologue->header |= 32;
        prologue->header |= THIS_BLOCK_ALLOCATED;

        //size_alloc += 32;

        sf_block *free_block = sf_mem_start()+32;
        free_block->header |= 4056;

        size_t size_block = free_block->header;
        size_block &= ~THIS_BLOCK_ALLOCATED;
        size_block &= ~PREV_BLOCK_ALLOCATED;
        size_block &= ~IN_QUICK_LIST;
        sf_footer *footer = (sf_footer*)((char*)free_block+size_block-8);
        *footer = (sf_footer)free_block->header;

        sf_block *epilogue = sf_mem_end()-8;
        epilogue->header |= 0;
        epilogue->header |= THIS_BLOCK_ALLOCATED;

        sf_block *head = sf_free_list_heads;
        head->body.links.next = head;
        head->body.links.prev = head;

 
        int min,max;
        for(int i = 0;i < 10; i++){

        if(i == 0){
            min = 16;
            max = 32;
            if(4056 == 32){
                sf_free_list_heads[i].body.links.next = free_block;
                free_block->body.links.prev = &sf_free_list_heads[i];
            }
            continue;
        }
        if(i == 9){
            sf_free_list_heads[i].body.links.prev = free_block;
            sf_free_list_heads[i].body.links.next = free_block;
            free_block->body.links.prev = &sf_free_list_heads[i];
            free_block->body.links.next = &sf_free_list_heads[i];
            break;
        }
        min = min * 2;
        max = max * 2;
        if(4056 > min && 4056 <= max){
            sf_free_list_heads[i].body.links.prev = free_block;
            sf_free_list_heads[i].body.links.next = free_block;
            free_block->body.links.prev = &sf_free_list_heads[i];
            free_block->body.links.next = &sf_free_list_heads[i];
            break;
        }
    }
    


    }

    sf_block block = {};
    sf_block *new_block=&block;
    new_block->header |= new_size;
    //sf_insert_free_block(new_block);
    

    //find free block from quick list
    sf_block *big_free_block = sf_search_free(new_block);


    //no space
    while(big_free_block == NULL ){
        //sf_show_heap();
        sf_footer *old_footer = sf_mem_end()-16;
        *old_footer &= ~0x7;
        sf_block *epilogue = sf_mem_end()-8;
        sf_block *old_free_block =(sf_block*) ((char*)epilogue-(size_t)*old_footer);
        *old_footer = 0;

        sf_block *grow = sf_mem_grow();
        if(grow == NULL){
            break;
        }

        epilogue = sf_mem_end()-8;
        epilogue->header = 0;

        epilogue = sf_mem_end()-8;
        epilogue->header |= 0;
        epilogue->header |= THIS_BLOCK_ALLOCATED;

        sf_block *prev = old_free_block->body.links.prev;
        sf_block *next = old_free_block->body.links.next;
            

        if((old_free_block->header & IN_QUICK_LIST) == 0){
            next->body.links.prev = prev;
            prev->body.links.next = next;
        }


        old_free_block->body.links.prev = NULL;
        old_free_block->body.links.next = NULL;


        old_free_block->header += 4096;
        //size_t size_block = old_free_block->header;
        sf_insert_free_block(old_free_block);

        size_t size_block = old_free_block->header;
        size_block &= ~THIS_BLOCK_ALLOCATED;
        size_block &= ~PREV_BLOCK_ALLOCATED;
        size_block &= ~IN_QUICK_LIST;
        sf_footer *footer = (sf_footer*)((char*)old_free_block+size_block-8);
        *footer = old_free_block->header;
        
        big_free_block = sf_search_free(new_block);


    }


    // IF FOUND
    if(big_free_block != NULL){
        size_t header = big_free_block->header;
        header &= ~THIS_BLOCK_ALLOCATED;
        header &= ~PREV_BLOCK_ALLOCATED;
        header &= ~IN_QUICK_LIST;

        size_t old_size = header;//4056
        size_t new_size = old_size - new_block->header;//4000
        

        sf_block *prev = big_free_block->body.links.prev;
        sf_block *next = big_free_block->body.links.next;
        

        if((big_free_block->header & IN_QUICK_LIST) == 0){
            next->body.links.prev = prev;
            prev->body.links.next = next;
        }


        big_free_block->body.links.prev = NULL;
        big_free_block->body.links.next = NULL;

        (big_free_block)->header = new_size;
        sf_block *split_block = big_free_block;//56
        //size_t new_block_header = new_block->header;
        size_t size_s = new_block->header;
        new_block->header |= THIS_BLOCK_ALLOCATED;
        new_block->header |= PREV_BLOCK_ALLOCATED;
        split_block->header = new_block->header;


        
        sf_block *heap_free = (sf_block*)((char*)split_block+size_s);//88
        heap_free->header = 0;
        heap_free->header |= new_size;

        

        sf_insert_free_block(heap_free);

        size_t size_block = heap_free->header;
        heap_free->header |= PREV_BLOCK_ALLOCATED;
        size_block &= ~THIS_BLOCK_ALLOCATED;
        size_block &= ~PREV_BLOCK_ALLOCATED;
        size_block &= ~IN_QUICK_LIST;
        sf_footer *footer = (sf_footer*)((char*)heap_free+size_block-8);
        *footer = heap_free->header;

        ptr = split_block;
        return ptr;
    }
    sf_errno = ENOMEM;
    return ptr;
    abort();
}

void sf_insert_free_block(sf_block *new_block){
    //size_t m = 32;
    size_t quick = 24;
    sf_header header = new_block->header;
    header &= ~THIS_BLOCK_ALLOCATED;
    new_block->header &= ~THIS_BLOCK_ALLOCATED;
    header &= ~PREV_BLOCK_ALLOCATED;
    header &= ~IN_QUICK_LIST;
    size_t new_size = header;
    int min,max;
    if(new_size < 32){return;}
    for(int i = 0; i<20; i++){
        quick+=8;
        if(quick >= new_size && sf_quick_lists[i].length != 5){
            sf_block *current = sf_quick_lists[i].first;

            for(int i = 0; i < sf_quick_lists[i].length-1; i++){
                current = current->body.links.next;
            }
            sf_quick_lists[i].length +=1;

            if(sf_quick_lists[i].length == 1){

                sf_quick_lists[i].first = new_block;
                sf_quick_lists[i].first->header |= IN_QUICK_LIST;
                sf_quick_lists[i].first->header |= THIS_BLOCK_ALLOCATED;
                return;
            }
            current->body.links.next = new_block;
            current->header |= IN_QUICK_LIST;
            current->header |= THIS_BLOCK_ALLOCATED;
            return;
        }
    }
    //put the new block into the free_list
    for(int i = 0;i < 10; i++){
        sf_block *first_block = &sf_free_list_heads[i];
        sf_block *current = &sf_free_list_heads[i];
        if(i == 0){
            min = 16;
            max = 32;
            if(new_size == 32){
                while(current->body.links.next != first_block){
                    current = (current->body.links.next);
                }
                sf_block *next = current->body.links.next;//first
                current->body.links.next = new_block;
                new_block->body.links.prev = current;
                next->body.links.prev = new_block;
                new_block->body.links.next = next;
                break;
            }
            continue;
        }
        if(i == 9){
            while(current->body.links.next != first_block){
                current = (current->body.links.next);
            }
            sf_block *next = current->body.links.next;//first
            current->body.links.next = new_block;
            new_block->body.links.prev = current;
            next->body.links.prev = new_block;
            new_block->body.links.next = next;
            break;
        }
        min = min * 2;
        max = max * 2;
        if(new_size > min && new_size <= max){
            while(current->body.links.next != first_block){
                current = (current->body.links.next);
            }
            sf_block *next = current->body.links.next;//first
            current->body.links.next = new_block;
            new_block->body.links.prev = current;
            next->body.links.prev = new_block;
            new_block->body.links.next = next;
            break;
        }
    }
}

sf_block *sf_search_free(sf_block *new_block){
    size_t q_size = 24;
    for(int i = 0; i < 20;i++){
        q_size += 8;
        if(new_block->header <= q_size){
            sf_block *current = sf_quick_lists[i].first;
            sf_block *prev = sf_quick_lists[i].first;
            int counter = 0;
            if(sf_quick_lists[i].first == NULL){continue;}
            while (current->body.links.next != NULL){
                if(counter != 0){
                    prev = current;
                }
                current = current->body.links.next;
                counter++;
            }
            if(current == sf_quick_lists[i].first ){
                sf_quick_lists[i].first = NULL;
            }
            else{
                prev->body.links.next = current->body.links.next;;
            }
            return current;
        }
    }
    //find free block from free list
    //sf_show_heap();
    for(int i = 0; i < 10; i++){
        sf_block *first_block = &sf_free_list_heads[i];
        sf_block *current = &sf_free_list_heads[i];
        current = (current)->body.links.next;
        while(current != first_block){
            if(current->header >= new_block->header){
                return current;
            }
            current = (current)->body.links.next;
        }
    }
    return NULL;
}

void sf_free(void *pp) {
    // TO BE IMPLEMENTED
    if(pp == NULL){
        abort();
    }

    sf_block *block = (sf_block*) pp;
    sf_header header = block->header;
    if ((header & IN_QUICK_LIST) != 0) {
        abort();
    }
    if ((header & THIS_BLOCK_ALLOCATED) == 0) {
        abort();
    }
    if ((header & PREV_BLOCK_ALLOCATED) == 0) {
        if((block-8)->header != 0){
            abort();
        }
    }
    header &= ~THIS_BLOCK_ALLOCATED;
    header &= ~PREV_BLOCK_ALLOCATED;
    header &= ~IN_QUICK_LIST;
    if(header % 8 != 0 || header < 32){
        abort();
    }

    if((sf_block*)sf_mem_start() >= block || (sf_block*)sf_mem_end() <= block){
        abort();
    }

    block = (sf_block*) pp;
    //sf_show_heap();
    

    sf_footer *prev_footer = (sf_footer*)((char*)block-8);
    *prev_footer &= ~0x7;
    if(*prev_footer != 0){// prev is free combine
        sf_block *prev_free_block = (sf_block*)((char*)block-(char*)prev_footer);

        sf_block *prev = prev_free_block->body.links.prev;
        sf_block *next = prev_free_block->body.links.next;
            

        if((prev_free_block->header & IN_QUICK_LIST) == 0){
            next->body.links.prev = prev;
            prev->body.links.next = next;
        }


        prev_free_block->body.links.prev = NULL;
        prev_free_block->body.links.next = NULL;

        prev_free_block->header += header;
        block->header = 0;
        *prev_footer = 0;
        block = prev_free_block;
    }

    size_t size_block = block->header;
    size_block &= ~0x7;
    sf_header *next_header  = (sf_header*)((char*)block+size_block);
    if((*next_header & THIS_BLOCK_ALLOCATED) == 0){// next is free combine
        sf_block *next_free_block = (sf_block*)((char*)block+size_block);

        sf_block *prev = next_free_block->body.links.prev;
        sf_block *next = next_free_block->body.links.next;
            

        if((next_free_block->header & IN_QUICK_LIST) == 0){
            next->body.links.prev = prev;
            prev->body.links.next = next;
        }


        next_free_block->body.links.prev = NULL;
        next_free_block->body.links.next = NULL;

        sf_footer *old_footer = (sf_footer*)((char*)block+size_block-8);
        *old_footer = 0;
        block->header += (next_free_block->header &= ~0x7);
        next_free_block->header = 0;
    }
    sf_insert_free_block(block);

    if((block->header & IN_QUICK_LIST)==0){
        size_block = block->header;
        size_block &= ~THIS_BLOCK_ALLOCATED;
        size_block &= ~PREV_BLOCK_ALLOCATED;
        size_block &= ~IN_QUICK_LIST;
        sf_footer *footer = (sf_footer*)((char*)block+size_block-8);
        *footer = block->header;

        sf_block *next_block = (sf_block*)((char*)block+size_block+8);
        next_block->header &= ~PREV_BLOCK_ALLOCATED;
    }
    

   
}

void *sf_realloc(void *pp, size_t rsize) {
    // TO BE IMPLEMENTED
    if(pp == NULL){
        sf_errno = EINVAL;
        return NULL;
    }
    sf_block *block = (sf_block*) pp;
    sf_header header = block->header;
    if ((header & IN_QUICK_LIST) != 0) {
        abort();
    }
    if ((header & THIS_BLOCK_ALLOCATED) == 0) {
        abort();
    }
    if ((header & PREV_BLOCK_ALLOCATED) == 0) {
        if((block-8)->header != 0){
            abort();
        }
    }
    header &= ~THIS_BLOCK_ALLOCATED;
    header &= ~PREV_BLOCK_ALLOCATED;
    header &= ~IN_QUICK_LIST;
    if(header % 8 != 0 || header < 32){
        abort();
    }
    if((sf_block*)sf_mem_start() >= block || (sf_block*)sf_mem_end() <= block){
        abort();
    }
    if(rsize == 0){
        sf_free(block);
        return NULL;
    }
    sf_block *ptr = NULL;
    
    
    size_t payload = rsize+8;
    size_t block_size = payload;//64
    while(block_size % 8 != 0){
        block_size++;
    }
    //if cant split
    size_t block_header = block->header;
    block_header &= ~0x7;
    if((block_header-block_size)<32){
        ptr = sf_malloc(block_header-8);
    
        if(ptr == NULL){return NULL;}
        memcpy(ptr,pp,rsize);
        sf_free(pp);
        //size_t size = ptr->header;
        //size &= ~0x7;
        //pp = ptr;
        //printf("\npp:%p,%p\n",pp,ptr);
        
        return ptr;
    }
    sf_block *allocated_block = ptr;
    sf_block *free_block = allocated_block+block_size;
    free_block->header |= block_header-block_size;
    free_block->header |= PREV_BLOCK_ALLOCATED;
    memcpy(ptr,pp,rsize);
    sf_free(free_block);
    sf_free(pp);
    //size_t size = ptr->header;
    //size &= ~0x7;
    
    pp = ptr;
    return ptr;
    abort();
}


void *sf_memalign(size_t size, size_t align) {
    void* ptr = sf_malloc(size + align - 1);
    if (ptr == NULL) {
      return NULL;
    }
    void* aligned_ptr = (void*) (((size_t) ptr + align - 1) & ~(align - 1));
    if (aligned_ptr != ptr) {
      size_t offset = (size_t) ((size_t) aligned_ptr - (size_t) ptr);
      *((size_t*) aligned_ptr - 1) = offset;
    }
  return aligned_ptr;
}
