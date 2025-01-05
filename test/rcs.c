#include <stdio.h>
#include <unistd.h>
#include "modeling_rcs.h"

int main() {
	printf("\n--- run 'modeling_rcs' ---\n\n");
	modeling_rcs("./test/test1.json", "./run_analysis/out1.ffi");
	modeling_rcs("./test/test2.json", "./run_analysis/out2.ffi");
	modeling_rcs("./test/test3.json", "./run_analysis/out3.ffi");
	return 0;
}