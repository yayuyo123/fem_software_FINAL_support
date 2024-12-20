#ifndef MODELING_DATA_H
#define MODELING_DATA_H

#include <stdio.h>

// 境界点の要素数
#define BOUNDARY_X_MAX 9
#define BOUNDARY_Y_MAX 5
#define BOUNDARY_Z_MAX 7

// x,y,z方向を指定する
typedef enum {
    DIR_X = 0,
    DIR_Y = 1,
    DIR_Z = 2,
} Direction;

// 境界点の情報を格納する配列の要素番号
typedef enum {
    // x境界
	BEAM_START_X             = 0,  // 梁端
    JIG_BEAM_X               = 1,  // 治具、梁
	BEAM_COLUMN_X            = 2,  // 梁、柱
    COLUMN_ORTHOGONAL_BEAM_X = 3,
	COLUMN_CENTER_X          = 4,  // x軸中心
    ORTHOGONAL_BEAM_COLUMN_X = 5,
	COLUMN_BEAM_X            = 6,  // 柱、梁
	BEAM_JIG_X               = 7,  // 梁、治具
	BEAM_END_X               = 8,  // 梁端
    // y境界
	COLUMN_SURFACE_START_Y   = 9,  // 柱の面
    COLUMN_BEAM_Y            = 10,  // 柱、梁
    CENTER_Y                 = 11,  // y軸中心
    BEAM_COLUMN_Y            = 12,  // 梁、柱
    COLUMN_SURFACE_END_Y     = 13,  // 柱の面 
    // z境界
	COLUMN_START_Z           = 14,  // 柱端
    JIG_COLUMN_Z             = 15,  // 治具、柱
    COLUMN_BEAM_Z            = 16,  // 柱、梁
    CENTAR_Z                 = 17,  // z軸中心
    BEAM_COLUMN_Z            = 18,  // 梁、柱
    COLUMN_JIG_Z             = 19,  // 柱、治具
    COLUMN_END_Z             = 20   // 柱端
} BoundaryType;

/**
 * NodeCoordinate構造体
 * 
 * 節点が存在する座標を格納する構造体。
 * 配列の最初の要素は0.0が格納される。
 * 
 * メンバ:
 * - node_num: 配列の要素数（節点の数）。
 * - coordinate: 各節点の座標値を格納する動的配列。
 *               配列のサイズは node_num に一致する。
 */
typedef struct {
    int node_num;     // 配列の要素数
    double* coordinate;   // 動的配列
} NodeCoordinate;

// 節点番号、要素番号
typedef struct {
    int node;
    int element;
}NodeElement;

// 柱の情報を格納する
typedef struct {
    NodeElement increment[3];
    NodeElement occupied_indices;
    NodeElement head;
} ColumnHexa;

// 主筋の位置を表す要素番号
typedef struct {
    int x;
    int y;
} RebarPositionIndex;

// 主筋付着
typedef struct {
    NodeElement increment;
    NodeElement occupied_indices;
    NodeElement occupied_indices_single;
    NodeElement head;
} RebarLine;

// 主筋情報
typedef struct {
    int rebar_num;
    RebarPositionIndex* positions;
    NodeElement increment;
    NodeElement occupied_indices;
    NodeElement occupied_indices_single;  // 1本の主筋
    NodeElement head;
} RebarFiber;

// 接合部鋼板
typedef struct {
    int increment_element[3];
    NodeElement occupied_indices;
    NodeElement head;
} JointQuad;

// 接合部付着
typedef struct {
    int occupied_indices;
    int head;
} JointFilm;

// 梁
typedef struct {
    NodeElement increment[3];
    NodeElement head;
} BeamHexaQuad;


// モデリングに用いるデータを格納する
typedef struct {
    NodeCoordinate* x; // x方向の節点座標
    NodeCoordinate* y; // y方向の節点座標
    NodeCoordinate* z; // z方向の節点座標

    // 境界点
	int boundary_index[BOUNDARY_X_MAX + BOUNDARY_Y_MAX + BOUNDARY_Z_MAX];

    // 柱
    ColumnHexa column_hexa;

    // 主筋
    RebarFiber* rebar_fiber;

    // 主筋付着
    RebarLine rebar_line;

    // 接合部鋼板
    JointQuad joint_quad;

    // 接合部フィルム要素
    JointFilm joint_film;
    
    // 梁
    BeamHexaQuad beam;

} ModelingData;

NodeCoordinate* allocate_node_coordinate(int num_nodes);

RebarFiber* allocate_rebar_fiber(int rebar_num);

ModelingData* allocate_modeling_data();

void initialize_node_coordinate(NodeCoordinate* node);

void initialize_rebar_fiber(RebarFiber* rebar);

void initialize_modeling_data(ModelingData* data);

int free_node_coordinate(NodeCoordinate* node);

int free_rebar_fiber(RebarFiber* rebar);

int free_modeling_data(ModelingData* data);

ModelingData* create_modeling_data(int x_node_num, int y_node_num, int z_node_num, int rebar_num);

void print_indent_md(int level);

void print_node_coordinate(const char* name, NodeCoordinate* node, int indent);

void print_node_element(NodeElement* elem, int indent);

void print_column_hexa(const char* name, ColumnHexa* column, int indent);

void print_rebar_fiber(const char* name, RebarFiber* rebar, int indent);

void print_rebar_line(const char* name, RebarLine* rebar_line, int indent);

void print_joint_quad(const char* name, JointQuad* rebar_line, int indent);

void print_joint_film(const char* name, JointFilm* joint_film, int indent);

void print_beam(const char* name, BeamHexaQuad* beam, int indent);

void print_modeling_data(ModelingData* data);

#endif