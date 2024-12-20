#include <stdio.h>
#include <stdlib.h>
#include "modeling_data.h"

/**
 * 動的確保の動作確認
 * -> 動的確保、初期化、解放
 */

// NodeCoordinateのテスト
int test1() {
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

// 
int test2() {
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

int test() {
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

int main() {
	printf("Hi\n");
	printf("------------------------------------------------\n");

	test();

	return 0;
}