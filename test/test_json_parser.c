#include <stdio.h>
#include "json_parser.h"

void test_json_parser() {
	// ファイル名が正しい場合
	printf("file name is '../source.json'\n");
	// 初期化
    JsonData* first_data = new_json_data();
	JsonParserResult result = json_parser("../source.json", first_data);
	if (result == JSON_PARSER_SUCCESS) {
		print_Json_data(first_data);
	} else {
		printf("Failed to parse JSON.\n");
	}
	free_json_data(first_data);

	// ファイル名が空の場合
	printf("file name is ''\n");
	JsonData* second_data = new_json_data();
	result = json_parser("", second_data);
	if (result == JSON_PARSER_SUCCESS) {
		print_Json_data(second_data);
	} else {
		printf("Failed to parse JSON.\n");
	}
	free_json_data(second_data);

	// ファイル名が間違えている場合
	printf("file name is 'abc'\n");
	JsonData* third_data = new_json_data();
	result = json_parser("abc", third_data);
	if (result == JSON_PARSER_SUCCESS) {
		print_Json_data(third_data);
	} else {
		printf("Failed to parse JSON.\n");
	}
	free_json_data(third_data);

	// 空データを渡した場合
	printf("input blank data\n");
	JsonData* blank_data = new_json_data();
	print_Json_data(blank_data);
	free_json_data(blank_data);

}

int main() {
	test_json_parser();
	return 0;
}