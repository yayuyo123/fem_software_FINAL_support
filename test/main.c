#include <stdio.h>
#include "test.h"

int main() {
	printf("start test\n");

	test_json_parser();
	test_modeling_data();

	return 0;
}