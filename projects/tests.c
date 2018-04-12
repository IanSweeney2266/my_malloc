/* Test script for my malloc */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

int t1_general();
int t2_defrag();
int t3_realloc();
int t4_moving_realloc();

void p_print(void* p);
void s_print(char *s);

int main(int argc, char* argv[]) {

	t4_moving_realloc();
	// print_list();
	return 0;
}

int t1_general() {
	return 1;
}

int t2_defrag() {
	s_print("running t2_defrag");

	void *p1 = malloc(64);
	void *p2 = malloc(128);
	void *p3 = malloc(512);
	void *p4 = malloc(256);
	void *p5 = malloc(1010);

	free(p5);
	print_list();
	free(p4);
	print_list();
	free(p3);
	print_list();
	free(p2);
	print_list();
	free(p1);
	print_list();

	return 1;

}

int t3_realloc() {
	s_print("running t2_defrag");

	void *p1 = malloc(100);
	void *p2 = malloc(100);
	void *p3 = malloc(100);
	void *p4 = malloc(100);
	print_list();
	p1 = realloc(p1, 500);
	print_list();
	free(p4);
	print_list();
	p3 = realloc(p3, 150);
	print_list();

	p1 = realloc(NULL, 1024);
	p1 = realloc(p1, 0);
	free(p2);

	print_list();

	return 1;

}

int t4_moving_realloc() {
	int i = 0;
	void *p;
	
	p = malloc(1);
	for(i=2;i<=32;i++) {
		p = realloc(p,i);
		malloc(1);
	}

	// print_list();

	return 1;
}

void p_print(void *p) {
	char str[80] = {"\0"};
	int msg_size = 80;

	snprintf(str, msg_size, "%p\n\r", p);
	write(STDOUT_FILENO, str, msg_size);
}

void s_print(char *s) {
	char str[80] = {"\0"};
	int msg_size = 80;

	snprintf(str, msg_size, "%s\n\r", s);
	write(STDOUT_FILENO, str, msg_size);
}
