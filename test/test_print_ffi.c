#include <stdio.h>
#include "print_ffi.h"

/**
 * print_head_template()のテスト
 */
void test1() {
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


/**
 * FINALの文法にしたがってデータの入力を行うためのライブラリをテストする。
 * 
 * 確認したい事
 * データの書き込みの成功、失敗
 */
int main() {
	printf("Hi!\n");
	test1();
	return 0;
}