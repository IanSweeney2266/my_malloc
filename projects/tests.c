/* Test script for my malloc */
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
	void* p;
	p = malloc(1);
	void *p2 = malloc(2);

	print_list();
	return 0;
}
