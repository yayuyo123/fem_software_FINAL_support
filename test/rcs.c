#include <stdio.h>
#include <unistd.h>
#include "modeling_rcs.h"

int main() {
	printf("\n--- run 'modeling_rcs' ---\n\n");

	// ディレクトリ名
    const char *input_dir = "./test";
    const char *output_dir = "./run_analysis";

    // 入力ファイル名と出力ファイル名を配列で管理
    const char *filenames[] = {
        "test1",
        "test2",
        "test3",
        "test_min"
    };

    size_t file_count = sizeof(filenames) / sizeof(filenames[0]);

    // バッファを使って完全なパスを格納
    char input_path[256];
    char output_path[256];

    for (size_t i = 0; i < file_count; i++) {
        // フルパスを作成
        snprintf(input_path, sizeof(input_path), "%s/%s.json", input_dir, filenames[i]);
        snprintf(output_path, sizeof(output_path), "%s/%s.ffi", output_dir, filenames[i]);

        // ログと関数呼び出し
        printf("Processing: %s -> %s\n", input_path, output_path);
        modeling_rcs(input_path, output_path);
    }

	return 0;
}