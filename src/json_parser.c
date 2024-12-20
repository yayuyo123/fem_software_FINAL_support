#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parson.h" // Parsonライブラリのヘッダーファイルをインクルード
#include "json_parser.h"

/**
 * JsonData構造体を初期化する関数
 */
JsonData* new_json_data() {
    // メモリを確保
    JsonData* json_data = (JsonData*)malloc(sizeof(JsonData));
    if (json_data == NULL) {
        perror("JsonDataのメモリ確保に失敗しました");
        exit(EXIT_FAILURE);
    }

    // 構造体全体をゼロ初期化
    memset(json_data, 0, sizeof(JsonData));

    // 初期化: 主筋配列を仮の長さ1で確保
    json_data->rebar.rebar_num = 1;
    json_data->rebar.rebars = (RebarPosition*)malloc(sizeof(RebarPosition) * json_data->rebar.rebar_num);
    if (json_data->rebar.rebars == NULL) {
        perror("主筋配列のメモリ確保に失敗しました");
        free(json_data); // JsonData自体のメモリを解放
        exit(EXIT_FAILURE);
    }

    // 主筋配列を初期化 (x, y を double型に変更)
    json_data->rebar.rebars[0].x = 0.0; // double型で初期化
    json_data->rebar.rebars[0].y = 0.0; // double型で初期化

    // mesh_x の初期化
    json_data->mesh_x.mesh_num = 1; // 仮の長さ1
    json_data->mesh_x.lengths = (double*)malloc(sizeof(double) * json_data->mesh_x.mesh_num); // double型に変更
    if (json_data->mesh_x.lengths == NULL) {
        perror("mesh_x配列のメモリ確保に失敗しました");
        free(json_data->rebar.rebars); // 主筋配列を解放
        free(json_data);
        exit(EXIT_FAILURE);
    }
    json_data->mesh_x.lengths[0] = 0.0; // 初期化値

    // mesh_y の初期化
    json_data->mesh_y.mesh_num = 1; // 仮の長さ1
    json_data->mesh_y.lengths = (double*)malloc(sizeof(double) * json_data->mesh_y.mesh_num); // double型に変更
    if (json_data->mesh_y.lengths == NULL) {
        perror("mesh_y配列のメモリ確保に失敗しました");
        free(json_data->mesh_x.lengths); // 解放
        free(json_data->rebar.rebars);
        free(json_data);
        exit(EXIT_FAILURE);
    }
    json_data->mesh_y.lengths[0] = 0.0; // 初期化値

    // mesh_z の初期化
    json_data->mesh_z.mesh_num = 1; // 仮の長さ1
    json_data->mesh_z.lengths = (double*)malloc(sizeof(double) * json_data->mesh_z.mesh_num); // double型に変更
    if (json_data->mesh_z.lengths == NULL) {
        perror("mesh_z配列のメモリ確保に失敗しました");
        free(json_data->mesh_y.lengths); // 解放
        free(json_data->mesh_x.lengths);
        free(json_data->rebar.rebars);
        free(json_data);
        exit(EXIT_FAILURE);
    }
    json_data->mesh_z.lengths[0] = 0.0; // 初期化値

    return json_data;
}

/**
 * JSONファイルをパースして、JsonData構造体にデータを格納する関数
 * 
 * ファイル名がNULLの場合、エラーメッセージを表示して終了します。
 * 解析に失敗した場合もエラーメッセージを表示し、プログラムは終了します。
 * 正常にデータを解析できた場合、JsonData構造体を返します。
 *
 * @param file_name JSONファイルのパス
 * @param jsonData JSONファイルから解析したデータを格納するための構造体ポインタ
 * @return JsonData JSONファイルから解析したデータ
 */
JsonParserResult json_parser(const char *file_name, JsonData *jsonData) {
    // 引数が NULL の場合はエラーとして処理
    if (file_name == NULL) {
        fprintf(stderr, "Error: File name is not provided.\n");
        return JSON_PARSER_ERROR;  // 異常終了
    }

    // JSONファイルを解析してルートJSON値を取得
	// json_value_free()を忘れない
    JSON_Value *root_value = json_parse_file(file_name);

    // 解析エラーの場合の処理
    if (root_value == NULL) {
        fprintf(stderr, "Failed to open json file.\n");
        return JSON_PARSER_ERROR;  // 異常終了
    }

    // JSONオブジェクトを取得
    JSON_Object *root_object = json_value_get_object(root_value);

    // "column" フィールドの値がオブジェクトの場合、そのオブジェクトを取得
    JSON_Object *column_object = json_object_get_object(root_object, "column");
    if (column_object == NULL) {
        fprintf(stderr, "'column' is not an object or does not exist.\n");
        json_value_free(root_value);
        return JSON_PARSER_ERROR;  // 異常終了
    }

    // "beam" フィールドの値がオブジェクトの場合、そのオブジェクトを取得
    JSON_Object *beam_object = json_object_get_object(root_object, "beam");
    if (beam_object == NULL) {
        fprintf(stderr, "'beam' is not an object or does not exist.\n");
        json_value_free(root_value);
        return JSON_PARSER_ERROR;  // 異常終了
    }


	// column構造体
    jsonData->column.span = (double)json_object_get_number(column_object, "span");
    jsonData->column.width = (double)json_object_get_number(column_object, "width");
    jsonData->column.depth = (double)json_object_get_number(column_object, "depth");
    jsonData->column.center_x = (double)json_object_get_number(column_object, "center_x");
    jsonData->column.center_y = (double)json_object_get_number(column_object, "center_y");
    jsonData->column.center_z = (double)json_object_get_number(column_object, "center_z");
    jsonData->column.compressive_strength = (double)json_object_get_number(column_object, "compressive_strength");

    // beam構造体
    jsonData->beam.span = (double)json_object_get_number(beam_object, "span");
    jsonData->beam.width = (double)json_object_get_number(beam_object, "width");
    jsonData->beam.depth = (double)json_object_get_number(beam_object, "depth");
    jsonData->beam.center_x = (double)json_object_get_number(beam_object, "center_x");
    jsonData->beam.center_y = (double)json_object_get_number(beam_object, "center_y");
    jsonData->beam.center_z = (double)json_object_get_number(beam_object, "center_z");
    jsonData->beam.orthogonal_beam_width = (double)json_object_get_number(beam_object, "orthogonal_beam_width");


	// "rebar" フィールドの値がオブジェクトの場合、そのオブジェクトを取得
    JSON_Array* rebars_array = json_object_get_array(root_object, "rebars");
    if (rebars_array == NULL) {
        fprintf(stderr, "'rebars' is not an object or does not exist.\n");
        json_value_free(root_value);
        return JSON_PARSER_ERROR;  // 異常終了
    }

	// rebar構造体
	// 配列の要素数をカウント
	jsonData->rebar.rebar_num = (int)json_array_get_count(rebars_array);

	// Rebarの動的メモリ確保
    jsonData->rebar.rebars = (RebarPosition *)malloc(jsonData->rebar.rebar_num * sizeof(RebarPosition));
    if (jsonData->rebar.rebars == NULL) {
        json_value_free(root_value);
        return JSON_PARSER_ERROR;  // 異常終了
    }

	// 主筋の位置データ
	for(int i = 0; i < jsonData->rebar.rebar_num; i++) {
		// 配列から、そのオブジェクトを取得
        JSON_Object* rebar_position_object = json_array_get_object(rebars_array, i);

		if (rebar_position_object == NULL) {
			fprintf(stderr, "'rebars position object' is not an object or does not exist.\n");
			json_value_free(root_value);
			return JSON_PARSER_ERROR;  // 異常終了
		}
		jsonData->rebar.rebars[i].x = (double)json_object_get_number(rebar_position_object, "x");
		jsonData->rebar.rebars[i].y = (double)json_object_get_number(rebar_position_object, "y");
	}

    // mesh_xの配列を取得 --------------------------------------------------------------------------------
    JSON_Array* mesh_x_array = json_object_get_array(root_object, "mesh_x");
    if(mesh_x_array == NULL) {
        fprintf(stderr, "Error: 'mesh_x' array not found in the JSON data.\n");
        json_value_free(root_value);
        return JSON_PARSER_ERROR;  // 異常終了
    }

    // 配列の要素数を取得
    jsonData->mesh_x.mesh_num = (int)json_array_get_count(mesh_x_array);

    // lengthsの動的メモリ確保
    jsonData->mesh_x.lengths = (double *)malloc(jsonData->mesh_x.mesh_num * sizeof(double));
    if (jsonData->mesh_x.lengths == NULL) {
        json_value_free(root_value);
        return JSON_PARSER_ERROR;  // 異常終了
    }
    // データの格納
    for(int i = 0; i < jsonData->mesh_x.mesh_num; i++) {
        jsonData->mesh_x.lengths[i] = (double)json_array_get_number(mesh_x_array, i);  // 数値を取得して格納
    }

    // mesh_yの配列を取得 --------------------------------------------------------------------------------
    JSON_Array* mesh_y_array = json_object_get_array(root_object, "mesh_y");
    if(mesh_y_array == NULL) {
        fprintf(stderr, "Error: 'mesh_y' array not found in the JSON data.\n");
        json_value_free(root_value);
        return JSON_PARSER_ERROR;  // 異常終了
    }

    // 配列の要素数を取得
    jsonData->mesh_y.mesh_num = (int)json_array_get_count(mesh_y_array);

    // lengthsの動的メモリ確保
    jsonData->mesh_y.lengths = (double *)malloc(jsonData->mesh_y.mesh_num * sizeof(double));
    if (jsonData->mesh_y.lengths == NULL) {
        json_value_free(root_value);
        return JSON_PARSER_ERROR;  // 異常終了
    }
    // データの格納
    for(int i = 0; i < jsonData->mesh_y.mesh_num; i++) {
        jsonData->mesh_y.lengths[i] = (double)json_array_get_number(mesh_y_array, i);  // 数値を取得して格納
    }

    // mesh_zの配列を取得 --------------------------------------------------------------------------------
    JSON_Array* mesh_z_array = json_object_get_array(root_object, "mesh_z");
    if(mesh_z_array == NULL) {
        fprintf(stderr, "Error: 'mesh_z' array not found in the JSON data.\n");
        json_value_free(root_value);
        return JSON_PARSER_ERROR;  // 異常終了
    }

    // 配列の要素数を取得
    jsonData->mesh_z.mesh_num = (int)json_array_get_count(mesh_z_array);

    // lengthsの動的メモリ確保
    jsonData->mesh_z.lengths = (double *)malloc(jsonData->mesh_z.mesh_num * sizeof(double));
    if (jsonData->mesh_z.lengths == NULL) {
        json_value_free(root_value);
        return JSON_PARSER_ERROR;  // 異常終了
    }
    // データの格納
    for(int i = 0; i < jsonData->mesh_z.mesh_num; i++) {
        jsonData->mesh_z.lengths[i] = (double)json_array_get_number(mesh_z_array, i);  // 数値を取得して格納
    }

    // 解析に使用したメモリを解放
    json_value_free(root_value);

    // 正常終了
    return JSON_PARSER_SUCCESS;
}


void free_json_data(JsonData* json_data) {
    if (json_data == NULL) return;

    // 主筋配列のメモリを解放
    if (json_data->rebar.rebars != NULL) {
        free(json_data->rebar.rebars);
        json_data->rebar.rebars = NULL;
    }

    // mesh_x 配列を解放
    if (json_data->mesh_x.lengths  != NULL) {
        free(json_data->mesh_x.lengths);
    }

    // mesh_y 配列を解放
    if (json_data->mesh_y.lengths  != NULL) {
        free(json_data->mesh_y.lengths);
    }

    // mesh_z 配列を解放
    if (json_data->mesh_z.lengths  != NULL) {
        free(json_data->mesh_z.lengths);
    }

    // JsonData自体のメモリを解放
    free(json_data);
}



/**
 * @brief 動的インデントを出力する関数
 * 
 * 指定されたレベル（階層）とスペース数に基づき、インデントを出力する。
 * 
 * @param level インデントの繰り返し回数（階層の深さ）
 * @param space_count 1回のインデントで出力する空白の数（0以上の整数）
 */
void print_indent(int level, int space_count) {
    if (space_count < 0) {
        fprintf(stderr, "Error: space_count cannot be negative.\n");
        return; // 負数が指定された場合はエラー
    }

    for (int i = 0; i < level; i++) {
        for (int j = 0; j < space_count; j++) {
            printf(" "); // 空白を出力
        }
    }
}

/**
 * @brief JSON風にデータをフォーマットして出力する関数
 * 
 * JsonData構造体の内容をJSON風に整形して標準出力に表示する。
 * 階層ごとのインデントを動的に指定でき、読みやすいフォーマットで表示する。
 * 
 * @param data 表示するJsonData構造体へのポインタ
 *             - NULLが渡された場合、エラーメッセージを表示して終了。
 * @param indent インデントとして使用する文字列（例: "    " や "\t" など）
 *               - インデントは各階層ごとに繰り返されて表示される。
 * 
 * @note
 * - Column、Beam、Rebar、Mesh（X, Y, Z）のデータを順にJSON風に出力する。
 * - Rebarは配列として出力され、各要素は { "x": ..., "y": ... } の形式になる。
 * - Meshの長さリストは配列として出力され、カンマ区切りで表示される。
 * 
 * @example
 * JsonData data = {...};  // 適切に初期化されたデータ
 * const char *indent = "    ";  // 4スペースのインデント
 * print_json_data(&data, indent);
 */
void print_json_data(const JsonData *data, int indent) {
    if (data == NULL) {
        printf("Error: Null pointer passed to print_json_data.\n");
        return;
    }

    int level = 0;

    printf("{\n");
    level++;

    // columnの内容を表示
    print_indent(level, indent);
    printf("\"Column\": {\n");
    level++;
    print_indent(level, indent);
    printf("\"Span\": %.2lf,\n", data->column.span);
    print_indent(level, indent);
    printf("\"Width\": %.2lf,\n", data->column.width);
    print_indent(level, indent);
    printf("\"Depth\": %.2lf,\n", data->column.depth);
    print_indent(level, indent);
    printf("\"Center_X\": %.2lf,\n", data->column.center_x);
    print_indent(level, indent);
    printf("\"Center_Y\": %.2lf,\n", data->column.center_y);
    print_indent(level, indent);
    printf("\"Center_Z\": %.2lf,\n", data->column.center_z);
    print_indent(level, indent);
    printf("\"Compressive_Strength\": %.2lf\n", data->column.compressive_strength);
    level--;
    print_indent(level, indent);
    printf("},\n");

    // beamの内容を表示
    print_indent(level, indent);
    printf("\"Beam\": {\n");
    level++;
    print_indent(level, indent);
    printf("\"Span\": %.2lf,\n", data->beam.span);
    print_indent(level, indent);
    printf("\"Width\": %.2lf,\n", data->beam.width);
    print_indent(level, indent);
    printf("\"Depth\": %.2lf,\n", data->beam.depth);
    print_indent(level, indent);
    printf("\"Center_X\": %.2lf,\n", data->beam.center_x);
    print_indent(level, indent);
    printf("\"Center_Y\": %.2lf,\n", data->beam.center_y);
    print_indent(level, indent);
    printf("\"Center_Z\": %.2lf,\n", data->beam.center_z);
    print_indent(level, indent);
    printf("\"Orthogonal_Beam_Width\": %.2lf\n", data->beam.orthogonal_beam_width);
    level--;
    print_indent(level, indent);
    printf("},\n");

    // rebarの内容を表示
    print_indent(level, indent);
    printf("\"Rebar\": [\n");
    level++;
    for (int i = 0; i < data->rebar.rebar_num; i++) {
        print_indent(level, indent);
        printf("{ \"x\": %.2lf, \"y\": %.2lf }", data->rebar.rebars[i].x, data->rebar.rebars[i].y);
        if (i < data->rebar.rebar_num - 1) {
            printf(",");
        }
        printf("\n");
    }
    level--;
    print_indent(level, indent);
    printf("],\n");

    // meshの内容を表示
    print_indent(level, indent);
    printf("\"Mesh_X\": {\n");
    level++;
    print_indent(level, indent);
    printf("\"Mesh_Num\": %d,\n", data->mesh_x.mesh_num);
    print_indent(level, indent);
    printf("\"Lengths\": [");
    for (int i = 0; i < data->mesh_x.mesh_num; i++) {
        printf("%.2lf", data->mesh_x.lengths[i]);
        if (i < data->mesh_x.mesh_num - 1) {
            printf(", ");
        }
    }
    printf("]\n");
    level--;
    print_indent(level, indent);
    printf("},\n");

    // 同様にMesh_YとMesh_Zも
    print_indent(level, indent);
    printf("\"Mesh_Y\": {\n");
    level++;
    print_indent(level, indent);
    printf("\"Mesh_Num\": %d,\n", data->mesh_y.mesh_num);
    print_indent(level, indent);
    printf("\"Lengths\": [");
    for (int i = 0; i < data->mesh_y.mesh_num; i++) {
        printf("%.2lf", data->mesh_y.lengths[i]);
        if (i < data->mesh_y.mesh_num - 1) {
            printf(", ");
        }
    }
    printf("]\n");
    level--;
    print_indent(level, indent);
    printf("},\n");

    print_indent(level, indent);
    printf("\"Mesh_Z\": {\n");
    level++;
    print_indent(level, indent);
    printf("\"Mesh_Num\": %d,\n", data->mesh_z.mesh_num);
    print_indent(level, indent);
    printf("\"Lengths\": [");
    for (int i = 0; i < data->mesh_z.mesh_num; i++) {
        printf("%.2lf", data->mesh_z.lengths[i]);
        if (i < data->mesh_z.mesh_num - 1) {
            printf(", ");
        }
    }
    printf("]\n");
    level--;
    print_indent(level, indent);
    printf("}\n");

    level--;
    printf("}\n");
}