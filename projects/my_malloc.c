#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
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

		return head + HEAD_SIZE;
	}
	/* Otherwise find the next available memory */
	else {
		memblk *cur = head;
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

		return cur + HEAD_SIZE;
	}

	return NULL;
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


