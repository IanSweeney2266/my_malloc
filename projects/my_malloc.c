#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#define HEAP_INCREMENT (64*1024)
#define HEAD_SIZE 20

void *calloc(size_t nmemb, size_t size);
void *malloc(size_t size);
void free(void *ptr);
void *realloc(void *ptr, size_t size);
 
typedef struct memblk_s {
	size_t size;
	int free;
	struct memblk_s * next;
} memblk;

memblk *head = NULL;
memblk *end = NULL;

void *malloc(size_t size) {
	/* Need to align*/
	char* dbg = getenv("DEBUG_MALLOC");

	memblk *cur = head;
	/* If this is the first malloc*/
	if (head == NULL) {
		/* Create the linked list */
		if ((head = sbrk(0)) == -1) {
			/* Err msg */
			errno = ENOMEM;
			return NULL;
		}
		/* Allocate a large block of memory */
		while (head + HEAD_SIZE + size > sbrk(0)) {
			/* Extend heap */
			if (sbrk(HEAP_INCREMENT) == -1) {
				/* Err msg */
				errno = ENOMEM;
				return NULL;
			}
		}	

		/* Finish creating the head */
		head->free = 0;
		head->size = size; /* Need to align size */
		head->next = head + HEAD_SIZE + size;
		end = head->next;
		end->free = 1;

		/* Pointer to where data goes */
		cur = head + HEAD_SIZE;
	}
	/* Otherwise find the next available memory */
	else {
		while (cur->free == 0 || cur->size < size) {
			cur = cur->next;
		}

		if (cur != end) {
			/* Reallocating a freed block */
			if (cur->size - size > HEAD_SIZE + 1) {
				/* make more free space */
				memblk *temp = cur->next;
				cur->next = cur + HEAD_SIZE + size;
				cur->next->size = cur->size - size - HEAD_SIZE;
				cur->next->free = 1;
				cur->next->next = temp;
			}

			cur->size = size;
			cur->free = 0;
		}

		else {
			while (cur + HEAD_SIZE + size > sbrk(0)) {
				/* Extend heap */
				if (sbrk(HEAP_INCREMENT) == -1) {
					/* Err msg */
					errno = ENOMEM;
					return NULL;
				}
			}

			/* Add a Node to the list */
			cur->size = size;
			cur->free = 0;
			cur->next = cur + HEAD_SIZE + size;
			end = cur->next;
		}

		/* Pointer to where data goes */
		cur = cur + HEAD_SIZE;
	}

	/* MALLOC: malloc(%d) => (ptr=%p, size=%d) */
	char str[80] = {"\0"};
	int msg_size = 80;
	
	snprintf(str, msg_size, 
		"MALLOC: malloc(%d)	=> (ptr=%p, size=%d)\n\r", 
		(int)size, cur, (int)size);
	write(STDOUT_FILENO, str, msg_size);
	return cur;
}

void *calloc(size_t nmemb, size_t size) {
	return NULL;
}

void free(void *ptr) {
	return;
}

void *realloc(void *ptr, size_t size) {
	return NULL;
}

void print_list() {
	memblk * view = head;

	char str[80] = {"\0"};
	int msg_size = 80;
	
	while (view != NULL) {
		snprintf(str, msg_size, 
			"(ptr=%p, size=%d, free=%d)\n", 
			view, (int)view->size, view->free);
		write(STDOUT_FILENO, str, msg_size);

		view = view->next;
	}
	
	return;
}


