#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>


#define ADDR_WIDTH 8
#define HEAP_INCREMENT (64*1024)
/* #define HEAD_SIZE (sizeof(struct memblk_s)) */
#define HEAD_SIZE 32

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
void print_list();


/* Global variables */
memblk *head = NULL;


/* Malloc family */
void *malloc(size_t size) {
	char* dbg = getenv("DEBUG_MALLOC");
	int dbg_flg = 0;

	size_t asize = align_16(size);
	memblk *cur = head;

	/* If this is the first malloc*/
	if (head == NULL) {
		
		/* Create the linked list */
		if ((head = sbrk(0)) == -1) {
			/* Err msg */
			errno = ENOMEM;
			return NULL;
		}

		/* Align the head */
		uintptr_t p = head;
		head = (memblk*) align_16(p);

		/* Allocate a large block of memory */
		while (head->data + asize > sbrk(0)) {
			/* Extend heap */
			if (sbrk(HEAP_INCREMENT) == -1) {
				/* Err msg */
				errno = ENOMEM;
				return NULL;
			}
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
			while (cur + cur->size + asize + 2*HEAD_SIZE > sbrk(0)) {
				/* Extend heap */
				if (sbrk(HEAP_INCREMENT) == -1) {
					/* Err msg */
					errno = ENOMEM;
					return NULL;
				}
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
	
	if (dbg_flg) {
		/* MALLOC: malloc(%d) => (ptr=%p, size=%d) */
		char str[80] = {"\0"};
		int msg_size = 80;

		snprintf(str, msg_size, 
			"MALLOC: malloc(%d)		=> (ptr=%p, size=%d)\n\r", 
			(int)size, cur, (int)asize);
		write(STDERR_FILENO, str, msg_size);
	}

	return cur->data;
}

void *calloc(size_t nmemb, size_t size) {

	char* dbg = getenv("DEBUG_MALLOC");
	int dbg_flg = 0;
	
	size_t asize = align_16(nmemb * size);
	if (asize == 0) {
		return NULL;
	}

	memblk *cur = malloc(asize);
	if (cur == NULL) {
		return NULL;
	}

	if (dbg_flg) {
		/* MALLOC: malloc(%d) => (ptr=%p, size=%d) */
		char str[80] = {"\0"};
		int msg_size = 80;

		snprintf(str, msg_size, 
			"MALLOC: calloc(%d,%d)	=> (ptr=%p, size=%d)\n\r", 
			(int)nmemb, (int)size, cur, (int)asize);
		write(STDERR_FILENO, str, msg_size);
	}
	
	return memset(cur->data, 0, asize);
}

void free(void *ptr) {

	char* dbg = getenv("DEBUG_MALLOC");
	int dbg_flg = 0;
	

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
			cur = cur->next;
		}

		/* combine with next if free */
		if (cur->next && cur->next->free) {
			cur = merge_blocks(cur, cur->next);	
		}
	}

	if (dbg_flg) {
		/* MALLOC: free(%p) */
		char str[80] = {"\0"};
		int msg_size = 80;
		
		snprintf(str, msg_size, "MALLOC: free(%p)\n\r", cur);
		write(STDERR_FILENO, str, msg_size);
	}

	return;
}

void *realloc(void *ptr, size_t size) {

	char* dbg = getenv("DEBUG_MALLOC");
	int dbg_flg = 0;

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
				
		/* Malloc and copy data */ 
		else {
			memblk *temp = cur;
			if ((cur = malloc(asize)) == NULL) {
				return NULL;
			}
			memcpy(cur->data, temp->data, asize);
			/* Need to not print twice */
			free(temp);
		}
	}

	
	if (dbg_flg) {
		/* MALLOC: realloc(%p,%d) => (ptr=%p, size=%d) */
		char str[80] = {"\0"};
		int msg_size = 80;
		
		snprintf(str, msg_size, 
			"MALLOC: realloc(%p,%d)	=> (ptr=%p, size=%d)\n\r", 
			ptr, (int)size, cur, (int)asize);
		write(STDERR_FILENO, str, msg_size);
	}
	
	return cur;
}


/* Helper functions */
memblk *split_block(memblk *ptr, size_t size) {
	if (ptr->size - size > HEAD_SIZE + 1) {
		/* make more free space */
		memblk *temp = ptr->next;
		ptr->next = ptr->data + size;
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

memblk *find_block(void *ptr) {
	if (ptr == NULL) {
		return NULL;
	}

	memblk *cur = head;
	while (cur && ((cur->data + cur->size) < ptr)) {
		cur = cur->next;
	}

	return cur;
}

void print_list() {
	memblk * view = head;

	char str[80] = {"\0"};
	int msg_size = 80;

	while (view != NULL) {
		snprintf(str, msg_size, 
			"(ptr=%p, size=%d, free=%d)\n", 
			view, (int)view->size, view->free);
		write(STDERR_FILENO, str, msg_size);

		view = view->next;
	}
	
	return;
}


