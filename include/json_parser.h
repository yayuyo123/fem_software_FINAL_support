#ifndef JSON_PARSER_H
#define JSON_PARSER_H

// 柱データ
typedef struct {
	double span;
	double width;
	double depth;
	double center_x;
	double center_y;
	double center_z;
	double compressive_strength;
} Column;

// 梁データ
typedef struct {
	double span;
	double width;
	double depth;
	double center_x;
	double center_y;
	double center_z;
	double orthogonal_beam_width;
} Beam;

// 主筋の位置
typedef struct {
    double x;
    double y;
} RebarPosition;

typedef struct {
    int rebar_num;
    RebarPosition *rebars;  // 動的配列
} Rebar;

typedef struct {
    int mesh_num;          // 配列の要素数
    double* lengths;   // 動的配列 (length, length_y, length_z に対応)
} Mesh;

// JsonData構造体の定義
typedef struct {
	Column column;
	Beam beam;
	Rebar rebar;
	Mesh mesh_x;
	Mesh mesh_y;
	Mesh mesh_z;
} JsonData;

typedef enum {
	JSON_PARSER_SUCCESS = 0,  // JSONのパース成功
    JSON_PARSER_ERROR = 1     // JSONのパース失敗
} JsonParserResult;

// JsonDataの初期化
JsonData* new_json_data();

// JsonDataにjsonファイルからデータ取得
JsonParserResult json_parser(const char *file_name, JsonData *jsonData);

// JsonDataのメモリを解放する
void free_json_data(JsonData *jsonData);

// JsonDataを表示する関数
void print_Json_data(const JsonData *data);

#endif