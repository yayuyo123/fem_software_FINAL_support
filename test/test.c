#include <stdio.h>
#include <stdlib.h>

/**
 * 動的確保に関する注意
 * - 構造体ごとにテストを行う
 * - 解放したらNULLポインタにすることを確認
 */


// json_parserのテスト ----------------------------------------------------------------------
#include "json_parser.h"

/**
 * JsonData構造体の初期化、データ格納、メモリの解放、表示する関数を確認する。
 */
void test_json_parser() {
	printf("--- 'test_json_parser' ---\n");
	const int indent = 4;  // 4スペース
	// ファイル名が正しい場合
	printf("file name is '../test/test.json'\n");
	// 初期化
    JsonData* first_data = new_json_data();
	JsonParserResult result = json_parser("../test/test.json", first_data);
	if (result == JSON_PARSER_SUCCESS) {
		print_json_data(first_data, indent);
	} else {
		printf("Failed to parse JSON.\n");
	}
	free_json_data(first_data);

	// ファイル名が空の場合
	printf("file name is ''\n");
	JsonData* second_data = new_json_data();
	result = json_parser("", second_data);
	if (result == JSON_PARSER_SUCCESS) {
		print_json_data(second_data, indent);
	} else {
		printf("Failed to parse JSON.\n");
	}
	free_json_data(second_data);

	// ファイル名が間違えている場合
	printf("file name is 'abc'\n");
	JsonData* third_data = new_json_data();
	result = json_parser("abc", third_data);
	if (result == JSON_PARSER_SUCCESS) {
		print_json_data(third_data, indent);
	} else {
		printf("Failed to parse JSON.\n");
	}
	free_json_data(third_data);

	// 空データを渡した場合
	printf("input blank data\n");
	JsonData* blank_data = new_json_data();
	print_json_data(blank_data, indent);
	free_json_data(blank_data);
}

// modeling_dataのテスト ----------------------------------------------------------------------
#include "modeling_data.h"

// NodeCoordinateのテスト
int test_node_coordinate() {
	// 動的確保
	const int node_number = 1;
	NodeCoordinate* node = allocate_node_coordinate(node_number);
	if(node == NULL) {
		printf("NodeCoordinate allocation failed\n");
		return 1;
	}
	printf("allocated NodeCoordinate Struct address -> %p\n", (void *)node);
	print_node_coordinate("test1", node, 1);

	// 初期化
	initialize_node_coordinate(node);
	printf("initialized NodeCoordinate Struct address -> %p\n", (void *)node);
	print_node_coordinate("test1", node, 1);

	// 解放
	if(free_node_coordinate(node) == EXIT_SUCCESS) {
		printf("success\n");
	} else {
		printf("failure\n");
	}	

	return 0;
}

// RebarFiber
int test_rebar_fiber() {
	const int rebar_num = 1;
	// 動的確保
	RebarFiber* rebar_fiber = allocate_rebar_fiber(rebar_num);
	if(rebar_fiber == NULL) {
		printf("RebarFiber allocation failed\n");
		return 1;
	}
	print_rebar_fiber("test2", rebar_fiber, 1);

	// 初期化
	initialize_rebar_fiber(rebar_fiber);
	print_rebar_fiber("test2", rebar_fiber, 1);

	// 解放
	if(free_rebar_fiber(rebar_fiber) == EXIT_SUCCESS) {
		printf("success\n");
	} else {
		printf("failure\n");
		return 1;
	}	
	return 0;
}

int test_modeling_data() {
	test_node_coordinate();
	test_rebar_fiber();
	
	// ModelingData構造体関連
	// 動的確保 + 初期化
	ModelingData *data = create_modeling_data(3, 1, 5, 5);
	if(data == NULL) {
		printf("ModelingData allocation failed\n");
		return 1;
	}
	print_modeling_data(data);

	// 解放
	if(free_modeling_data(data) == EXIT_SUCCESS) {
		printf("success\n");
	} else {
		printf("failure\n");
		return 1;
	}	
	return 0;
}

#include "print_ffi.h"

/**
 * print_head_template()のテスト
 */
void test_head_template() {
	// 正しい入力
	print_head_template(stdout, 10, 1, 'x', 1, 'x');
	// ストリーム
	print_head_template(NULL, 10, 1, 'x', 0, 'x');
	// 第2引数
	print_head_template(stdout, -1, 1, 'x', 1, 'x');
	print_head_template(stdout, 0, 1, 'x', 1, 'x');
	print_head_template(stdout, 123456, 1, 'x', 1, 'x');
	// 第3引数
	print_head_template(stdout, 1, -1, 'x', 1, 'x');
	print_head_template(stdout, 10, 0, 'x', 1, 'x');
	print_head_template(stdout, 10, 123456, 'x', 1, 'x');

}

#include "modeling_rcs.h"

void test_modeling_rcs() {
	modeling_rcs("../test/test.json", "../run_analysis/out.ffi");
}