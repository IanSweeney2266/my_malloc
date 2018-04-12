/** @file my_malloc.c
 *  @brief My version of the malloc(3) functions
 *	
 *	This contains functions for handling memory allocation
 *
 *	@author imsweene
 */

#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>


#define MSG_SIZE 80
#define HEAP_INCREMENT (64*1024)
/* #define HEAD_SIZE (sizeof(struct memblk_s)) */
#define HEAD_SIZE (sizeof(struct memblk_s) - sizeof(char*))

#define align_16(x) (((((x)-1)>>4)<<4)+16)


/* Memory block structure */
typedef struct memblk_s {
	size_t size;
	int free;
	struct memblk_s * prev;
	struct memblk_s * next;
	char data[1];
} memblk;


/* Function prototypes */
void *calloc(size_t nmemb, size_t size);
void *malloc(size_t size);
void free(void *ptr);
void *realloc(void *ptr, size_t size);

memblk *split_block(memblk *ptr, size_t size);
memblk *merge_blocks(memblk *ptr, memblk *next);
memblk *find_block(void *ptr);
void extend_heap();
void print_list();


/* Global variables */
memblk *head = NULL;


/* Malloc family */

/*
* @brief Allocates memory.
* @param size the amount of memory to allocate
* @return a pointer to the first byte of data
*/
void *malloc(size_t size) {
	char* dbg = getenv("DEBUG_MALLOC");

	size_t asize = align_16(size);
	memblk *cur = head;
	uintptr_t p;

	/* If this is the first malloc*/
	if (head == NULL) {
		
		/* Create the linked list */
		if ((uintptr_t)(head = sbrk(0)) == -1) {
			/* Err msg */
			errno = ENOMEM;
			exit(1);
		}

		/* Align the head */
		p = (uintptr_t)head;
		head = (memblk*) align_16(p);

		/* Allocate a large block of memory */
		while (head->data + asize > (char*)sbrk(0)) {
			extend_heap();
		}

		/* Finish creating the head */
		head->free = 0;
		head->size = asize;
		head->next = NULL;
		head->prev = NULL;

		/* Pointer to header for new data */
		cur = head;
	}
	/* Otherwise find the next available memory */
	else {
		while (cur->next && (cur->free == 0 || cur->size < asize)) {
			cur = cur->next;
		}

		/* Reallocating a freed block */
		if (cur->next) {
			cur = split_block(cur, asize);
		}

		/* Creating a new block */
		else {
			while (cur->data + cur->size + 
				asize + HEAD_SIZE > (char*)sbrk(0)) {

				extend_heap();
			}

			/* Add a Node to the list */
			cur->next = (memblk*) (cur->data + cur->size);
			cur->next->prev = cur;
			cur = cur->next; /* move ptr to new block */
			cur->size = asize;
			cur->free = 0;
			cur->next = NULL;
			/*end = cur->next;*/
		}

	}
	
	if (dbg) {
		/* MALLOC: malloc(%d) => (ptr=%p, size=%d) */
		char str[MSG_SIZE] = {"\0"};

		snprintf(str, MSG_SIZE, 
			"MALLOC: malloc(%d)	=> (ptr=%p, size=%d)\n\r", 
			(int)size, cur, (int)asize);
		write(STDERR_FILENO, str, MSG_SIZE);
	}

	return cur->data;
}

/*
* @brief Allocates and clears memory.
* @param size the size of a member
* @param nmemb the number of members
* @return a pointer to the first byte of data
*/
void *calloc(size_t nmemb, size_t size) {
	char* dbg = getenv("DEBUG_MALLOC");
	
	size_t asize = align_16(nmemb * size);
	if (asize == 0) {
		return NULL;
	}

	memblk *cur = malloc(asize);
	cur = (memblk*)((uintptr_t)cur - HEAD_SIZE);
	if (cur == NULL) {
		return NULL;
	}

	if (dbg) {
		/* MALLOC: malloc(%d) => (ptr=%p, size=%d) */
		char str[MSG_SIZE] = {"\0"};

		snprintf(str, MSG_SIZE, 
			"MALLOC: calloc(%d,%d)	=> (ptr=%p, size=%d)\n\r", 
			(int)nmemb, (int)size, cur, (int)asize);
		write(STDERR_FILENO, str, MSG_SIZE);
	}
	
	return memset(cur->data, 0, asize);
}

/*
* @brief Frees memory.
* @param ptr a pointer to somewhere in the block of memory
* 	to be freed
*/
void free(void *ptr) {

	char* dbg = getenv("DEBUG_MALLOC");
	

	if (ptr == NULL) {
		return;
	}

	memblk *cur = find_block(ptr);
	if (!cur) {
		return;
	}

	else {
		cur->free = 1;
		
		/* combine with prev if free */
		if (cur->prev && cur->prev->free) {
			cur = merge_blocks(cur->prev, cur);
		}

		/* combine with next if free */
		if (cur->next && cur->next->free) {
			cur = merge_blocks(cur, cur->next);	
		}
	}

	if (dbg) {
		/* MALLOC: free(%p) */
		char str[MSG_SIZE] = {"\0"};
		
		snprintf(str, MSG_SIZE, "MALLOC: free(%p)\n\r", cur);
		write(STDERR_FILENO, str, MSG_SIZE);
	}

	return;
}

/*
* @brief Reallocates memory at ptr to a block of size size.
* @param ptr a pointer to somewhere in the block of memory
* @param size the amount of memory to allocate
* @return a pointer to the first byte of data
*/
void *realloc(void *ptr, size_t size) {

	char* dbg = getenv("DEBUG_MALLOC");

	size_t asize = align_16(size);

	memblk *cur = find_block(ptr);
	if (!cur) {
		/* Need to not print twice */
		return malloc(size);
	}
	else if (size == 0) {
		/* Need to not print twice */
		free(ptr);
		return NULL;
	}


	if (asize <= cur->size) {
		cur = split_block(cur, asize);
	}

	else {
		/* Try to extend the block */
		if(cur->next && cur->next->free && 
			(cur->size + cur->next->size + HEAD_SIZE) > asize) {
			cur = merge_blocks(cur, cur->next);
			cur = split_block(cur, asize);
		}

		/* Extend the last block */
		else if (!cur->next) {
			while (cur->data + asize > (char*)sbrk(0)) {
				extend_heap();
			}
			cur->size = asize;

		}
				
		/* Malloc and copy data */ 
		else {
			memblk *temp = cur;
			if ((cur = malloc(asize)) == NULL) {
				return NULL;
			}
			cur = (memblk*)((uintptr_t)cur - HEAD_SIZE);
			memcpy(cur->data, temp->data, asize);
			/* Need to not print twice */
			free(temp);
		}
	}

	
	if (dbg) {
		/* MALLOC: realloc(%p,%d) => (ptr=%p, size=%d) */
		char str[MSG_SIZE] = {"\0"};
		
		snprintf(str, MSG_SIZE, 
			"MALLOC: realloc(%p,%d)	=> (ptr=%p, size=%d)\n\r", 
			ptr, (int)size, cur, (int)asize);
		write(STDERR_FILENO, str, MSG_SIZE);
	}
	
	return cur->data;
}


/* Helper functions */

/*
* @brief Splits a block of memory at size.
* @param ptr a pointer to the start of the block
* @param size the amount of memory to allocate
* @return a pointer to the first byte of the header
*/
memblk *split_block(memblk *ptr, size_t size) {
	if (ptr->size - size > HEAD_SIZE + 1) {
		/* make more free space */
		memblk *temp = ptr->next;
		ptr->next = (memblk*) (ptr->data + size);
		ptr->next->size = ptr->size - size - HEAD_SIZE;
		ptr->next->free = 1;
		ptr->next->next = temp;
		temp->prev = ptr->next;

		/* So memory blocks smaller than HEAD_SIZE */ 
		/* don't get abbandoned */
		ptr->size = size;
	}

	ptr->free = 0;

	return ptr;
}

/*
* @brief Merges two blocks of memory.
* @param ptr a pointer to a header
* @param next a pointer to the next header
* @return a pointer to the first byte of the header
*/
memblk *merge_blocks(memblk *ptr, memblk *next) {
	ptr->size += next->size + HEAD_SIZE;
	if (next->next) {
		ptr->next = next->next;
		next->next->prev = ptr;
	}
	else {
		ptr->next = NULL;
	}

	return ptr;
}

/*
* @brief Finds a header given a pointer somewhere in its block.
* @param ptr a pointer to a location in a block
* @return a pointer to the first byte of the header
*/
memblk *find_block(void *ptr) {
	if (ptr == NULL) {
		return NULL;
	}

	memblk *cur = head;
	while (cur && ((cur->data + cur->size) < (char*)ptr)) {
		cur = cur->next;
	}

	return cur;
}

/*
* @brief Extends the heap by HEAP_INCREMENT
*/
void extend_heap() {
	if ((uintptr_t)sbrk(HEAP_INCREMENT) == -1) {
		/* Err msg */
		errno = ENOMEM;
		perror("Sbrk");
		exit(1);
	}
}

/*
* @brief Prints each block in the linked list
*/
void print_list() {
	memblk * view = head;

	char str[MSG_SIZE] = {"\0"};

	while (view != NULL) {
		snprintf(str, MSG_SIZE, 
			"(ptr=%p, size=%d, free=%d)\n", 
			view, (int)view->size, view->free);
		write(STDERR_FILENO, str, MSG_SIZE);

		view = view->next;
	}
	
	return;
}


