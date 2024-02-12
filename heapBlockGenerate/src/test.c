#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>


#include "myMalloc.h"
#include "printing.h"

void
mymalloc_info_to_file(FILE *fp, char* filename) {
	freopen(filename, "w", stdout);
	tags_print(print_object);
	freopen("/dev/tty", "w", stdout);
}

void mymalloc_info(FILE *fp) {
	tags_print(print_object);
}

int main() {
	FILE * fp;
	my_malloc(128);

	// mymalloc_info(fp);
	const char* filename = "../results/output.txt";
	// mymalloc_info_to_file(fp, filename);
	mymalloc_info(fp);

	return 0;
}


