#include <stdio.h>
#include <stdlib.h>
#include "json_parser.h"
#include "function.h"
#include "modeling_rcs.h"
#include "print_ffi.h"
#include "modeling_data.h"

/**
 * source_dataからモデリングに必要なデータを作成し、modeling_dayaに格納する
 */
int make_modeling_data(ModelingData* modeling_data, JsonData *source_data) {

    // x軸方向の節点座標
    modeling_data->x->coordinate[0] = 0;
    for(int i = 1; i < modeling_data->x->node_num; i++) {
        modeling_data->x->coordinate[i] = modeling_data->x->coordinate[i - 1] + source_data->mesh_x.lengths[i - 1];
    }
    // y方向
    modeling_data->y->coordinate[0] = 0;
    for(int i = 1; i < modeling_data->y->node_num; i++) {
        modeling_data->y->coordinate[i] = modeling_data->y->coordinate[i - 1] + source_data->mesh_y.lengths[i - 1];
    }
    // z方向
    modeling_data->z->coordinate[0] = 0;
    for(int i = 1; i < modeling_data->z->node_num; i++) {
        modeling_data->z->coordinate[i] = modeling_data->z->coordinate[i - 1] + source_data->mesh_z.lengths[i - 1];
    }

    const int jig_element_num = 1;  // 端部からの治具の要素数
    double target_coordinate = 0.0;

    // 境界点
    // x軸方向 ------------------------------------------------------------
    modeling_data->boundary_index[BEAM_START_X] = 0;
    // 治具、梁
    modeling_data->boundary_index[JIG_BEAM_X] = jig_element_num;
    // 梁、柱
    target_coordinate = source_data->column.center_x - source_data->column.depth / 2;
    modeling_data->boundary_index[BEAM_COLUMN_X] = find_index_double(modeling_data->x->coordinate, modeling_data->x->node_num, target_coordinate);
    // 柱、直交梁
    target_coordinate = source_data->column.center_x - source_data->beam.orthogonal_beam_width / 2;
    modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] = find_index_double(modeling_data->x->coordinate, modeling_data->x->node_num, target_coordinate);
    // x軸方向、柱芯
    target_coordinate = source_data->column.center_x;
    modeling_data->boundary_index[COLUMN_CENTER_X] = find_index_double(modeling_data->x->coordinate, modeling_data->x->node_num, target_coordinate);
    // 直交梁、柱
    target_coordinate = source_data->column.center_x + source_data->beam.orthogonal_beam_width / 2;
    modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] = find_index_double(modeling_data->x->coordinate, modeling_data->x->node_num, target_coordinate);
    // 柱、梁
    target_coordinate = source_data->column.center_x + source_data->column.depth / 2;
    modeling_data->boundary_index[COLUMN_BEAM_X] = find_index_double(modeling_data->x->coordinate, modeling_data->x->node_num, target_coordinate);
    // 梁、治具
    modeling_data->boundary_index[BEAM_JIG_X] = modeling_data->x->node_num - 1 - jig_element_num;
    // 梁端
    modeling_data->boundary_index[BEAM_END_X] = modeling_data->x->node_num - 1;

    // y軸方向 ---------------------------------------------------------------------
    // 柱の面
    modeling_data->boundary_index[COLUMN_SURFACE_START_Y] = 0;
    // 柱、梁
    target_coordinate = source_data->beam.center_y - source_data->beam.width / 2;
    modeling_data->boundary_index[COLUMN_BEAM_Y] = find_index_double(modeling_data->y->coordinate, modeling_data->y->node_num, target_coordinate);
    // 梁芯
    target_coordinate = source_data->beam.center_y;
    modeling_data->boundary_index[CENTER_Y] = find_index_double(modeling_data->y->coordinate, modeling_data->y->node_num, target_coordinate);
    // 梁、柱
    target_coordinate = source_data->beam.center_y + source_data->beam.width / 2;
    modeling_data->boundary_index[BEAM_COLUMN_Y] = find_index_double(modeling_data->y->coordinate, modeling_data->y->node_num, target_coordinate);
    // 柱の面
    modeling_data->boundary_index[COLUMN_SURFACE_END_Y] = modeling_data->y->node_num - 1;

    // z軸方向 ---------------------------------------------------------------------------
    // 柱端
    modeling_data->boundary_index[COLUMN_START_Z] = 0;
    // 治具、柱
    modeling_data->boundary_index[JIG_COLUMN_Z] = jig_element_num;
    // 柱、梁
    target_coordinate = source_data->beam.center_z - source_data->beam.depth / 2;
    modeling_data->boundary_index[COLUMN_BEAM_Z] = find_index_double(modeling_data->z->coordinate, modeling_data->z->node_num, target_coordinate);
    // 梁芯
    target_coordinate = source_data->beam.center_z;
    modeling_data->boundary_index[CENTAR_Z] = find_index_double(modeling_data->z->coordinate, modeling_data->z->node_num, target_coordinate);
    // 梁、柱
    target_coordinate = source_data->beam.center_z + source_data->beam.depth / 2;
    modeling_data->boundary_index[BEAM_COLUMN_Z] = find_index_double(modeling_data->z->coordinate, modeling_data->z->node_num, target_coordinate);
    // 柱、治具
    modeling_data->boundary_index[COLUMN_JIG_Z] = modeling_data->z->node_num - 1 - jig_element_num;
    // 柱端
    modeling_data->boundary_index[COLUMN_END_Z] = modeling_data->z->node_num - 1;

    // 境界点のエラーチェック
    for(int i = 0; i < BOUNDARY_X_MAX + BOUNDARY_Y_MAX + BOUNDARY_Z_MAX; i++) {
        if(modeling_data->boundary_index[i] == -1) {
            printf("Error: No matching index for target_coordinate\n");
            printf("     : function - 'make_modeling_data'\n");
            printf("     : boundary type [ %d ]\n", i);
            printf("     : index [ %d ]\n", modeling_data->boundary_index[i]);
            printf("     : coordinate [ %f ]\n", modeling_data->x->coordinate[modeling_data->boundary_index[i]]);
            return EXIT_FAILURE;
        }
    }

    // 柱 ------------------------------------------------------------------------
    // インクリメント
    modeling_data->column_hexa.increment[DIR_X].node = 1;
    modeling_data->column_hexa.increment[DIR_Y].node = (modeling_data->boundary_index[COLUMN_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 2);
    modeling_data->column_hexa.increment[DIR_Z].node = (modeling_data->boundary_index[COLUMN_SURFACE_END_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + 2) * modeling_data->column_hexa.increment[DIR_Y].node;
    modeling_data->column_hexa.increment[DIR_X].element = 1;
    modeling_data->column_hexa.increment[DIR_Y].element = (modeling_data->boundary_index[COLUMN_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X]);
    modeling_data->column_hexa.increment[DIR_Z].element = (modeling_data->boundary_index[COLUMN_SURFACE_END_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * modeling_data->column_hexa.increment[DIR_Y].element;
    // 必要節点、要素数
    modeling_data->column_hexa.occupied_indices.node = 
        (modeling_data->boundary_index[COLUMN_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 2) * 
        (modeling_data->boundary_index[COLUMN_SURFACE_END_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + 2) * 
        (modeling_data->boundary_index[COLUMN_END_Z] - modeling_data->boundary_index[COLUMN_START_Z] + 3);
    modeling_data->column_hexa.occupied_indices.element = 
        (modeling_data->boundary_index[COLUMN_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * 
        (modeling_data->boundary_index[COLUMN_SURFACE_END_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * 
        (modeling_data->boundary_index[COLUMN_END_Z] - modeling_data->boundary_index[COLUMN_START_Z]);
    // 始めの番号
    modeling_data->column_hexa.head.node = 1;
    modeling_data->column_hexa.head.element = 1;

    // 主筋 -------------------------------------------------------------------------
    if(modeling_data->rebar_fiber->rebar_num != source_data->rebar.rebar_num) {
        printf("rebar num not\n");
    }
    // x方向は柱までの長さを加算
    // y方向はそのまま
    double pin_column = modeling_data->x->coordinate[modeling_data->boundary_index[BEAM_COLUMN_X]];
    for(int i = 0; i < source_data->rebar.rebar_num; i++) {
        modeling_data->rebar_fiber->positions[i].x = find_index_double(modeling_data->x->coordinate, modeling_data->x->node_num, source_data->rebar.rebars[i].x + pin_column);
        modeling_data->rebar_fiber->positions[i].y = find_index_double(modeling_data->y->coordinate, modeling_data->y->node_num, source_data->rebar.rebars[i].y);
    }
    
    modeling_data->rebar_fiber->increment.node = modeling_data->column_hexa.increment[DIR_Z].node;
    modeling_data->rebar_fiber->increment.element = 1;

    modeling_data->rebar_fiber->occupied_indices_single.node =
        (modeling_data->boundary_index[COLUMN_JIG_Z] - modeling_data->boundary_index[JIG_COLUMN_Z] + 1) *
        modeling_data->rebar_fiber->increment.node;
    modeling_data->rebar_fiber->occupied_indices_single.element =
        (modeling_data->boundary_index[COLUMN_JIG_Z] - modeling_data->boundary_index[JIG_COLUMN_Z]) *
        modeling_data->rebar_fiber->increment.element;

    modeling_data->rebar_fiber->occupied_indices.node =
        modeling_data->rebar_fiber->occupied_indices_single.node * modeling_data->rebar_fiber->rebar_num;
    modeling_data->rebar_fiber->occupied_indices.element =
        modeling_data->rebar_fiber->occupied_indices_single.element * modeling_data->rebar_fiber->rebar_num;

    modeling_data->rebar_fiber->head.node = modeling_data->column_hexa.occupied_indices.node + modeling_data->column_hexa.head.node;
    modeling_data->rebar_fiber->head.element = modeling_data->column_hexa.occupied_indices.element + modeling_data->column_hexa.head.element;

    // ライン要素
    modeling_data->rebar_line.increment.node = 0;
    modeling_data->rebar_line.increment.element = 1;

    modeling_data->rebar_line.occupied_indices_single.node =
        (modeling_data->boundary_index[COLUMN_JIG_Z] - modeling_data->boundary_index[JIG_COLUMN_Z] + 1) *
        modeling_data->rebar_line.increment.node;
    modeling_data->rebar_line.occupied_indices_single.element = 
        (modeling_data->boundary_index[COLUMN_JIG_Z] - modeling_data->boundary_index[JIG_COLUMN_Z]) *
        modeling_data->rebar_line.increment.element;

    modeling_data->rebar_line.occupied_indices.node = modeling_data->rebar_line.occupied_indices_single.node * modeling_data->rebar_fiber->rebar_num;
    modeling_data->rebar_line.occupied_indices.element = modeling_data->rebar_line.occupied_indices_single.element * modeling_data->rebar_fiber->rebar_num;
    
    modeling_data->rebar_line.head.node = modeling_data->rebar_fiber->head.node + modeling_data->rebar_fiber->occupied_indices.node;
    modeling_data->rebar_line.head.element = modeling_data->rebar_fiber->head.element + modeling_data->rebar_fiber->occupied_indices.element;

    // 接合部 -------------------------------------------------------------------------
    // 節点は柱と同じ
    modeling_data->joint_quad.increment_element[DIR_X] = 1;
    modeling_data->joint_quad.increment_element[DIR_Y] = (modeling_data->boundary_index[COLUMN_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 4);
    modeling_data->joint_quad.increment_element[DIR_Z] = (modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + 2) * modeling_data->joint_quad.increment_element[DIR_Y];
    
    modeling_data->joint_quad.occupied_indices.node =
        (modeling_data->boundary_index[COLUMN_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 1) * modeling_data->column_hexa.increment[DIR_X].node +
        (modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + 1) * modeling_data->column_hexa.increment[DIR_Y].node +
        (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] + 1) * modeling_data->column_hexa.increment[DIR_Z].node;
    modeling_data->joint_quad.occupied_indices.element =
        (modeling_data->boundary_index[COLUMN_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 3) * modeling_data->joint_quad.increment_element[DIR_X] +
        (modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + 2) * modeling_data->joint_quad.increment_element[DIR_Y] +
        (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] + 2) * modeling_data->joint_quad.increment_element[DIR_Z];
    
    modeling_data->joint_quad.head.node = modeling_data->rebar_fiber->head.node + modeling_data->rebar_fiber->occupied_indices.node;
    modeling_data->joint_quad.head.element = modeling_data->rebar_line.head.element + modeling_data->rebar_line.occupied_indices.element;

    modeling_data->joint_film.head = modeling_data->joint_quad.head.element + modeling_data->joint_quad.occupied_indices.element;
    modeling_data->joint_film.occupied_indices =
        (modeling_data->boundary_index[COLUMN_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 4) * modeling_data->joint_quad.increment_element[DIR_X] +
        (modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + 2) * modeling_data->joint_quad.increment_element[DIR_Y] +
        (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] + 4) * modeling_data->joint_quad.increment_element[DIR_Z];
    // 梁 -------------------------------------------------------------------------
    modeling_data->beam.increment[DIR_X].node = 1;
    modeling_data->beam.increment[DIR_Y].node =
        (modeling_data->boundary_index[BEAM_END_X] - modeling_data->boundary_index[BEAM_START_X] + 1) * modeling_data->beam.increment[DIR_X].node;
    modeling_data->beam.increment[DIR_Z].node =
        (modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_BEAM_Y] + 1) * modeling_data->beam.increment[DIR_Y].node;
    
    modeling_data->beam.increment[DIR_X].element = 1;
    modeling_data->beam.increment[DIR_Y].element =
        (modeling_data->boundary_index[BEAM_END_X] - modeling_data->boundary_index[BEAM_START_X]) * modeling_data->beam.increment[DIR_X].element;
    modeling_data->beam.increment[DIR_Z].element =
        (modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_BEAM_Y] + 1) * modeling_data->beam.increment[DIR_Y].element;
    
    modeling_data->beam.head.node = modeling_data->joint_quad.head.node + modeling_data->joint_quad.occupied_indices.node;
    modeling_data->beam.head.element = modeling_data->joint_film.head + modeling_data->joint_film.occupied_indices;
    
    return EXIT_SUCCESS;
}


/**
 * 節点情報をプロットする関数
 * 
 * この関数は、指定された範囲内で与えられた増分に従い、節点の座標をプロットするためにファイルに書き込みます。
 * 座標は3次元空間（x, y, z）において、指定された範囲で順番に処理されます。 
 * また、NodeCoordinateの先頭アドレスを格納した3次元配列を参照して、各節点の座標を取得します。
 * 
 * @param f ファイルポインタ。プロット結果を出力するファイルを指す。
 * @param coordinates NodeCoordinateの先頭アドレスを格納する3次元配列のアドレス。
 * @param start_node_index 始めの節点番号。処理を開始する節点のインデックス。
 * @param start 始めの点の要素番号。3次元配列で、[x, y, z] の各座標の開始インデックスを指定。
 * @param end 終わりの点。各次元で、対応する座標の最大インデックスを指定。例：[x_end, y_end, z_end]
 * @param increment 節点番号の増分。各次元（x, y, z）における座標の増分を指定する3次元配列。
 *                  例：[x_increment, y_increment, z_increment] により各次元で増加する節点番号を定義。
 */
void plot_node(FILE *f, NodeCoordinate** coordinates, int start_node_index, int start[], int end[], int increment[]) {
    // dir : 0x, 1y, 2z
    int dir = 0;
    int pre_dir = 0;
    int index_delt = 0;

    // 節点定義
    print_NODE(f, start_node_index, coordinates[0]->coordinate[start[0]], coordinates[1]->coordinate[start[1]], coordinates[2]->coordinate[start[2]]);
    
    //節点コピー
    // 1次元コピー
    int index = start_node_index;
    for(dir = 0; dir < 3; dir++) {
        if(end[dir] > start[dir]) {
            for(int i = start[dir], cnt = 0; i < end[dir]; i += cnt) {
                cnt = count_consecutive(i, end[dir], coordinates[dir]->coordinate, coordinates[dir]->node_num);
                if(cnt < 0) {
                    printf("[ERROR] count_consecutive returned a negative value. i: %d, end[dir]: %d, dir: %d, node_num: %d\n", 
                        i, end[dir], dir, coordinates[dir]->node_num);
                    return ;
                }
                float length = coordinates[dir]->coordinate[i + 1] - coordinates[dir]->coordinate[i];
                print_COPYNODE(f, index, 0, 0, length, increment[dir], cnt, dir);
                index += cnt * increment[dir];
            }
            index_delt = index - start_node_index;
            pre_dir = dir;
            dir++;
            break;  // 2次元コピーへ
        }
    }
    // 2次元コピー
    for(; dir < 3; dir++) {
        if(end[dir] > start[dir]) {
            index = start_node_index;
            for(int i = start[dir], cnt = 0; i < end[dir]; i += cnt) {
                cnt = count_consecutive(i, end[dir], coordinates[dir]->coordinate, coordinates[dir]->node_num);
                if(cnt < 0) {
                    printf("[ERROR] Y\n");
                    return ;
                }
                float length = coordinates[dir]->coordinate[i + 1] - coordinates[dir]->coordinate[i];
                print_COPYNODE(f, index, index + index_delt, increment[pre_dir], length, increment[dir], cnt, dir);
                index += cnt * increment[dir];
            }
            dir++;
            break; // 3次元コピーへ
        }
    }
    // 3次元コピー
    if(dir < 3 && end[dir] > start[dir]) {
        // y方向コピー
        for(int j = start[1]; j <= end[1]; j++) {
            index = start_node_index + (j - start[1]) * increment[1];
            for(int i = start[2], cnt = 0; i < end[2]; i += cnt) {
                cnt = count_consecutive(i, end[2], coordinates[2]->coordinate, coordinates[2]->node_num);
                if(cnt < 0) {
                    printf("[ERROR] Z\n");
                    return ;
                }
                float length = coordinates[2]->coordinate[i + 1] - coordinates[2]->coordinate[i];
                print_COPYNODE(f, index, index + index_delt, increment[0], length, increment[2], cnt, 2);
                index += cnt * increment[2];
            }
        }
    }
    fprintf(f, "\n");
}

void generate_hexa(
    FILE *f,
    NodeCoordinate** coordinates,
    int start_node_index,
    int start_elm_index,
    int start[],
    int end[],
    int node_increment[],
    int element_increment[],
    int typh
) {
    //節点定義
    plot_node(f, coordinates, start_node_index, start, end, node_increment);
    int delt = 0;
    //要素定義
    print_HEXA_increment(f, start_elm_index, start_node_index, node_increment, typh);

    //要素コピー
    // x方向
    int element_set_x = end[0] - start[0] - 1;
    int element_set_y = end[1] - start[1] - 1;
    int element_set_z = end[2] - start[2] - 1;
    if(element_set_x > 0) {
        delt = element_set_x * element_increment[0];
        print_COPYELM(f, start_elm_index, 0, 0, element_increment[0], node_increment[0], element_set_x);
    }

    // y方向　
    if(element_set_y > 0) {
        if(element_set_x > 0) {
            print_COPYELM(f, start_elm_index, start_elm_index + delt, element_increment[0], element_increment[1], node_increment[1], element_set_y);
        } else {
            delt = element_set_y * element_increment[1];
            print_COPYELM(f, start_elm_index, 0, 0, element_increment[1], node_increment[1], element_set_y);
        }
    }

    // z方向
    if(element_set_z > 0) {
        if(element_set_y > 0 && element_set_x > 0) {
            for(int i = 0; i < end[1] - start[1]; i++) {
                print_COPYELM(f, start_elm_index + element_increment[1] * i, start_elm_index + element_increment[1] * i + delt, element_increment[0], element_increment[2], node_increment[2], element_set_z);
            }
        } else if(element_set_x > 0) {
            print_COPYELM(f, start_elm_index, start_elm_index + delt, element_increment[0], element_increment[2], node_increment[2], element_set_z);
        } else if(element_set_y > 0) {
            print_COPYELM(f, start_elm_index, start_elm_index + delt, element_increment[1], element_increment[2], node_increment[2], element_set_z);
        } else {
            print_COPYELM(f, start_elm_index, 0, 0, element_increment[2], node_increment[2], element_set_z);
        }
    }
    fprintf(f, "\n");
}

/**
 * 節点、要素番号を算出する
 */
int calculate_index(int start, int increment[], int dir_x, int dir_y, int dir_z) {
    return start + dir_x * increment[DIR_X] + dir_y * increment[DIR_Y] + dir_z * increment[DIR_Z];
}

/**
 * 柱、接合部の境界節点を定義する
 * 
 * @param f
 * @param start_nede 対称とする面の左下節点番号
 */
void column_joint_boundary_node(FILE *f, NodeCoordinate** coordinates, int node_increment[], const int *boundary_index, int origin_node_index, BoundaryType z_boundary_type) {
    // 直交フランジ部分、左側
    int start_node_index = origin_node_index + (boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - boundary_index[BEAM_COLUMN_X] + 1) * node_increment[DIR_X];
    int start[3] = {
        boundary_index[COLUMN_ORTHOGONAL_BEAM_X] + 1,
        boundary_index[COLUMN_SURFACE_START_Y],
        boundary_index[z_boundary_type]
    };
    int end[3] = {
        boundary_index[COLUMN_CENTER_X],
        boundary_index[COLUMN_BEAM_Y],
        boundary_index[z_boundary_type]
    };
    plot_node(f, coordinates, start_node_index, start, end, node_increment);

    // 直交フランジ部分、右側
    start_node_index = origin_node_index + (boundary_index[COLUMN_CENTER_X] - boundary_index[BEAM_COLUMN_X] + 1) * node_increment[DIR_X];
    start[0] = boundary_index[COLUMN_CENTER_X];
    end[0] = boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - 1;
    plot_node(f, coordinates, start_node_index, start, end, node_increment);

    // 梁フランジ、左
    start_node_index = origin_node_index + (boundary_index[COLUMN_BEAM_Y] - boundary_index[COLUMN_SURFACE_START_Y] + 1) * node_increment[DIR_Y];
    start[0] = boundary_index[BEAM_COLUMN_X];
    start[1] = boundary_index[COLUMN_BEAM_Y] + 1;
    end[0] = boundary_index[COLUMN_CENTER_X];
    end[1] = boundary_index[CENTER_Y];
    plot_node(f, coordinates, start_node_index, start, end, node_increment);

    // 梁フランジ、右
    start_node_index = 
        origin_node_index +
        (boundary_index[COLUMN_CENTER_X] - boundary_index[BEAM_COLUMN_X] + 1) * node_increment[DIR_X] +
        (boundary_index[COLUMN_BEAM_Y] - boundary_index[COLUMN_SURFACE_START_Y] + 1) * node_increment[DIR_Y];
    start[0] = boundary_index[COLUMN_CENTER_X];
    end[0] = boundary_index[COLUMN_BEAM_X];
    plot_node(f, coordinates, start_node_index, start, end, node_increment);
}

/**
 * RebarPositionIndex構造体のxまたわyの最大値を探す
 * add_columnのかぶりコンクリートを設定する為の補助関数
 */
int find_max_rebar_position_index(RebarPositionIndex* position, int position_num, Direction direction) {
    int max = -1;
    if(direction == DIR_X) {
        max = position[0].x;
        for(int i = 1; i < position_num; i++) {
            if(position[i].x > max) {
                max = position[i].x;
            }
        }
    } else if(direction == DIR_Y) {
        max = position[0].y;
        for(int i = 1; i < position_num; i++) {
            if(position[i].y > max) {
                max = position[i].y;
            }
        }
    } else {
        printf("dircion error\n");
        return -1;
    }
    return max;
}

/**
 * RebarPositionIndex構造体のxまたわyの最小値を探す
 * add_columnのかぶりコンクリートを設定する為の補助関数
 */
int find_min_rebar_position_index(RebarPositionIndex* position, int position_num, Direction direction) {
    int min = -1;
    if(direction == DIR_X) {
        min = position[0].x;
        for(int i = 1; i < position_num; i++) {
            if(position[i].x < min) {
                min = position[i].x;
            }
        }
    } else if(direction == DIR_Y) {
        min = position[0].y;
        for(int i = 1; i < position_num; i++) {
            if(position[i].y < min) {
                min = position[i].y;
            }
        }
    } else {
        printf("dircion error\n");
        return -1;
    }
    return min;
}

/**
 * 上下柱コンクリートを作成後、接合部の上下境界面より内側を作成し、接合部上下境界面を作成する。
 * 
 * 上部 (節点定義 -> 要素定義 -> 要素番号定義)
 * 下部 (節点定義 -> 要素定義 -> 要素番号定義)
 * 接合部 (
 *      上下端面を除いた部分 (generate_hexa)
 * )
 * 
 */
void add_column_hexa(FILE *f, ModelingData *modeling_data) {

    fprintf(f, "---- COLUMN HEXA ----\n");

    typedef struct {
        int column;        // 柱コンクリート
        int column_cover;  // 柱かぶりコンクリート
        int joint_inner;   // 接合部内部コンクリート
        int joint_outer;   // 接合部外部コンクリート
        int column_jig;    // 柱治具
    } Typh;

    Typh typh ={
        1,
        2,
        3,
        4,
        5
    };

    // ポインタ配列に各方向を格納
    NodeCoordinate* coordinates[3] = {modeling_data->x, modeling_data->y, modeling_data->z};

    int node_increment[3] = {
        modeling_data->column_hexa.increment[DIR_X].node,
        modeling_data->column_hexa.increment[DIR_Y].node,
        modeling_data->column_hexa.increment[DIR_Z].node
    };
    int element_increment[3] = {
        modeling_data->column_hexa.increment[DIR_X].element,
        modeling_data->column_hexa.increment[DIR_Y].element,
        modeling_data->column_hexa.increment[DIR_Z].element
    };

    // 柱、下部 -----------------------------------------------------
    int start[3] = {
        modeling_data->boundary_index[BEAM_COLUMN_X],
        modeling_data->boundary_index[COLUMN_SURFACE_START_Y],
        modeling_data->boundary_index[COLUMN_START_Z]
    };
    int end[3] = {
        modeling_data->boundary_index[COLUMN_BEAM_X],
        modeling_data->boundary_index[CENTER_Y],
        modeling_data->boundary_index[COLUMN_BEAM_Z]
    };

    fprintf(f, "---- lower\n");
    generate_hexa(
        f,
        coordinates,
        modeling_data->column_hexa.head.node,
        modeling_data->column_hexa.head.element,
        start,
        end,
        node_increment,
        element_increment,
        typh.column
    );
    
    // 治具要素番号
    int start_elmemnt_index = modeling_data->column_hexa.head.element;
    int end_element_index = start_elmemnt_index + (modeling_data->boundary_index[COLUMN_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X] - 1) * element_increment[DIR_X];
    int element_set = (modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] - 1);
    print_ETYP(f, start_elmemnt_index, end_element_index, element_increment[DIR_X], typh.column_jig, element_increment[DIR_Y], element_set);
    
    // かぶりコンクリート要素番号 x方向
    int x_max = find_max_rebar_position_index(modeling_data->rebar_fiber->positions, modeling_data->rebar_fiber->rebar_num, DIR_X);
    int x_min = find_min_rebar_position_index(modeling_data->rebar_fiber->positions, modeling_data->rebar_fiber->rebar_num, DIR_X);
    //int y_max = find_max_rebar_position_index(modeling_data->rebar_fiber.positions, modeling_data->rebar_fiber.rebar_num, DIR_Y);
    int y_min = find_min_rebar_position_index(modeling_data->rebar_fiber->positions, modeling_data->rebar_fiber->rebar_num, DIR_Y);

    for(int i = modeling_data->boundary_index[COLUMN_SURFACE_START_Y]; i < y_min; i++) {
        start_elmemnt_index = modeling_data->column_hexa.head.element +
            (modeling_data->boundary_index[JIG_COLUMN_Z] - modeling_data->boundary_index[COLUMN_START_Z]) * element_increment[DIR_Z] +
            i * element_increment[DIR_Y];
        end_element_index =
            start_elmemnt_index + (modeling_data->boundary_index[COLUMN_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X] - 1) * element_increment[DIR_X];
        element_set = (modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[JIG_COLUMN_Z] - 1);
        print_ETYP(f, start_elmemnt_index, end_element_index, element_increment[DIR_X], typh.column_cover, element_increment[DIR_Z], element_set);
    }

    // かぶりコンクリート要素番号 y方向
    for(int i = 0; i < x_min - modeling_data->boundary_index[BEAM_COLUMN_X]; i++) {
        start_elmemnt_index = modeling_data->column_hexa.head.element +
            (modeling_data->boundary_index[JIG_COLUMN_Z] - modeling_data->boundary_index[COLUMN_START_Z]) * element_increment[DIR_Z] +
            y_min * element_increment[DIR_Y] +
            i * element_increment[DIR_X];
        end_element_index =
            start_elmemnt_index + (modeling_data->boundary_index[CENTER_Y] - y_min) * element_increment[DIR_Y];
        element_set = (modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[JIG_COLUMN_Z] - 1);
        print_ETYP(f, start_elmemnt_index, end_element_index, element_increment[DIR_Y], typh.column_cover, element_increment[DIR_Z], element_set);
    }
    for(int i = x_max - modeling_data->boundary_index[BEAM_COLUMN_X]; i < modeling_data->boundary_index[COLUMN_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X]; i++) {
        start_elmemnt_index = modeling_data->column_hexa.head.element +
            (modeling_data->boundary_index[JIG_COLUMN_Z] - modeling_data->boundary_index[COLUMN_START_Z]) * element_increment[DIR_Z] +
            y_min * element_increment[DIR_Y] +
            i * element_increment[DIR_X];
        end_element_index =
            start_elmemnt_index + (modeling_data->boundary_index[CENTER_Y] - y_min) * element_increment[DIR_Y];
        element_set = (modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[JIG_COLUMN_Z] - 1);
        print_ETYP(f, start_elmemnt_index, end_element_index, element_increment[DIR_Y], typh.column_cover, element_increment[DIR_Z], element_set);
    }
    fprintf(f, "\n");

    // 柱、上部 -----------------------------------------------------
    int start_node_index = modeling_data->column_hexa.head.node + (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_START_Z] + 2) * node_increment[DIR_Z];
    start_elmemnt_index = modeling_data->column_hexa.head.element + (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_START_Z]) * element_increment[DIR_Z];

    start[2] = modeling_data->boundary_index[BEAM_COLUMN_Z];
    end[2] = modeling_data->boundary_index[COLUMN_END_Z];

    fprintf(f, "---- upper\n");
    generate_hexa(f, coordinates, start_node_index, start_elmemnt_index, start, end, node_increment, element_increment, typh.column);

    // 治具要素番号
    start_elmemnt_index = modeling_data->column_hexa.head.element + (modeling_data->boundary_index[COLUMN_JIG_Z] - modeling_data->boundary_index[COLUMN_START_Z]) * element_increment[DIR_Z];
    end_element_index = start_elmemnt_index + (modeling_data->boundary_index[COLUMN_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X] - 1) * element_increment[DIR_X];
    element_set = (modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] - 1);
    print_ETYP(f, start_elmemnt_index, end_element_index, element_increment[DIR_X], typh.column_jig, element_increment[DIR_Y], element_set);

    // かぶりコンクリート要素番号 x方向
    for(int i = modeling_data->boundary_index[COLUMN_SURFACE_START_Y]; i < y_min; i++) {
        start_elmemnt_index = modeling_data->column_hexa.head.element +
            (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_START_Z]) * element_increment[DIR_Z] +
            i * element_increment[DIR_Y];
        end_element_index =
            start_elmemnt_index + (modeling_data->boundary_index[COLUMN_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X] - 1) * element_increment[DIR_X];
        element_set = (modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[JIG_COLUMN_Z] - 1);
        print_ETYP(f, start_elmemnt_index, end_element_index, element_increment[DIR_X], typh.column_cover, element_increment[DIR_Z], element_set);
    }
    // かぶりコンクリート要素番号 y方向
    for(int i = 0; i < x_min - modeling_data->boundary_index[BEAM_COLUMN_X]; i++) {
        start_elmemnt_index = modeling_data->column_hexa.head.element +
            (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_START_Z]) * element_increment[DIR_Z] +
            y_min * element_increment[DIR_Y] +
            i * element_increment[DIR_X];
        end_element_index =
            start_elmemnt_index + (modeling_data->boundary_index[CENTER_Y] - y_min) * element_increment[DIR_Y];
        element_set = (modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[JIG_COLUMN_Z] - 1);
        print_ETYP(f, start_elmemnt_index, end_element_index, element_increment[DIR_Y], typh.column_cover, element_increment[DIR_Z], element_set);
    }
    for(int i = x_max - modeling_data->boundary_index[BEAM_COLUMN_X]; i < modeling_data->boundary_index[COLUMN_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X]; i++) {
        start_elmemnt_index = modeling_data->column_hexa.head.element +
            (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_START_Z]) * element_increment[DIR_Z] +
            y_min * element_increment[DIR_Y] +
            i * element_increment[DIR_X];
        end_element_index =
            start_elmemnt_index + (modeling_data->boundary_index[CENTER_Y] - y_min) * element_increment[DIR_Y];
        element_set = (modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[JIG_COLUMN_Z] - 1);
        print_ETYP(f, start_elmemnt_index, end_element_index, element_increment[DIR_Y], typh.column_cover, element_increment[DIR_Z], element_set);
    }
    fprintf(f, "\n");

    // 接合部 -----------------------------------------------------
    /**
     * 1,節点定義
     * 
     * 2,要素定義
     * - 上下端面を除いた接合部(接合部の要素がz方向に2つしかない場合、要素定義なし。)
     * - 上下端
     * 
     * 
     */
    // 接合部始めの節点番号(下柱上端)
    const int joint_origin_node = start_node_index = 
        modeling_data->column_hexa.head.node +
        (modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[COLUMN_START_Z]) * node_increment[DIR_Z];
    const int joint_origin_elememnt =
        modeling_data->column_hexa.head.element +
        (modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[COLUMN_START_Z]) * element_increment[DIR_Z];
    
    // 接合部 上下端を除いた節点
    // 右
    fprintf(f, "---- joint node\n");
    start_node_index = joint_origin_node + 2 * node_increment[DIR_Z];
    start[0] = modeling_data->boundary_index[BEAM_COLUMN_X];
    start[1] = modeling_data->boundary_index[COLUMN_SURFACE_START_Y];
    start[2] = modeling_data->boundary_index[COLUMN_BEAM_Z] + 1;

    end[0] = modeling_data->boundary_index[COLUMN_CENTER_X];
    end[1] = modeling_data->boundary_index[CENTER_Y];
    end[2] = modeling_data->boundary_index[BEAM_COLUMN_Z] - 1;

    plot_node(
        f,
        coordinates,
        start_node_index,
        start,
        end,
        node_increment
    );

    // 左
    start_node_index += (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 1) * modeling_data->column_hexa.increment[DIR_X].node;
    start[0] = modeling_data->boundary_index[COLUMN_CENTER_X];
    end[0] = modeling_data->boundary_index[COLUMN_BEAM_X];
    
    plot_node(
        f,
        coordinates,
        start_node_index,
        start,
        end,
        node_increment
    );

    // 柱と接合部の上下境界面の節点
    start_node_index = modeling_data->column_hexa.head.node + (modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[COLUMN_START_Z] + 1) * modeling_data->column_hexa.increment[2].node;
    fprintf(f, "---- joint lower node\n");
    column_joint_boundary_node(f, coordinates, node_increment, modeling_data->boundary_index, start_node_index, COLUMN_BEAM_Z);
    start_node_index = modeling_data->column_hexa.head.node + (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_START_Z] + 1) * modeling_data->column_hexa.increment[2].node;
    fprintf(f, "---- joint upper node\n");
    column_joint_boundary_node(f, coordinates, node_increment, modeling_data->boundary_index, start_node_index, BEAM_COLUMN_Z);

    // 接合部 上下端を除いた要素
    // 接合部のz方向要素数が2以下の場合、要素を定義しない
    if(modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] > 2) {
        for(int i = 0; i < 2; i++) {
            switch (i) {
            case 0:
                // 接合部、左
                fprintf(f, "---- joint left element\n");
                start_node_index = joint_origin_node + 2 * node_increment[DIR_Z];
                start_elmemnt_index = joint_origin_elememnt + element_increment[DIR_Z];

                start[0] = modeling_data->boundary_index[BEAM_COLUMN_X];
                start[1] = modeling_data->boundary_index[COLUMN_SURFACE_START_Y];
                start[2] = modeling_data->boundary_index[COLUMN_BEAM_Z] + 1;

                end[0] = modeling_data->boundary_index[COLUMN_CENTER_X];
                end[1] = modeling_data->boundary_index[CENTER_Y];
                end[2] = modeling_data->boundary_index[BEAM_COLUMN_Z] - 1;
                break;
            case 1:
                // 接合部、右
                fprintf(f, "---- joint right element\n");
                start_node_index += (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 1) * modeling_data->column_hexa.increment[DIR_X].node;
                start_elmemnt_index += (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * modeling_data->column_hexa.increment[DIR_X].element;
                start[0] = modeling_data->boundary_index[COLUMN_CENTER_X];
                end[0] = modeling_data->boundary_index[COLUMN_BEAM_X];
                break;
            default:
                // 例外
                break;
            }

            int delt;
            //要素定義
            print_HEXA_increment(f, start_elmemnt_index, start_node_index, node_increment, typh.joint_inner);
            //要素コピー
            //x
            print_COPYELM(f, start_elmemnt_index, 0, 0, element_increment[0], node_increment[0], end[0] - start[0] - 1);
            //y
            delt = (end[0] - start[0] - 1) * element_increment[0];
            print_COPYELM(f, start_elmemnt_index, start_elmemnt_index + delt, element_increment[0], element_increment[1], node_increment[1], end[1] - start[1] - 1);
            //z
            for(int i = 0; i < end[1] - start[1]; i++)
            {
                print_COPYELM(f, start_elmemnt_index + element_increment[1] * i, start_elmemnt_index + delt + element_increment[1] * i, element_increment[0], element_increment[2], node_increment[2], end[2] - start[2] - 1);
            }
            fprintf(f, "\n");
        }
    }
    
    // 接合部の上下面
    // 内部要素 - 直交梁フランジ
    start_node_index =
        modeling_data->column_hexa.head.node +
        (modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[COLUMN_START_Z] + 1) * modeling_data->column_hexa.increment[2].node +
        (modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 1) * modeling_data->column_hexa.increment[0].node;
    start_elmemnt_index = 
        modeling_data->column_hexa.head.element +
        (modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[COLUMN_START_Z]) * modeling_data->column_hexa.increment[2].element +
        (modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 1) * modeling_data->column_hexa.increment[0].element;
    end_element_index =
        modeling_data->column_hexa.head.element +
        (modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[COLUMN_START_Z]) * modeling_data->column_hexa.increment[2].element +
        (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X] - 2) * modeling_data->column_hexa.increment[0].element;
    // 左側最後の要素番号
    // この次節点番号 +1
    int node_change_element_index = 
        modeling_data->column_hexa.head.element +
        (modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[COLUMN_START_Z]) * modeling_data->column_hexa.increment[2].element +
        (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X] - 1) * modeling_data->column_hexa.increment[0].element;
    
    fprintf(f, "---- boundary inner orthogonal element\n");
    for(int i = start_elmemnt_index; i <= end_element_index;) {
        print_HEXA_increment(f, i, start_node_index, node_increment, typh.joint_inner);
        int set = modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] - 1;
        // 下面
        print_COPYELM(f, i, 0, 0, element_increment[1], node_increment[1], set);
        // 上面
        int elm_inc = (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1) * element_increment[2];
        int node_inc = (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1) * node_increment[2];
        print_COPYELM(f, i, i + element_increment[1] * set, element_increment[1], elm_inc, node_inc, 1);
        
        if(i == end_element_index) {
            printf("success\n");
            break;
        }
        
        // 直交ウェブを超えるときに更に節点番号を1増加
        if(i == node_change_element_index) {
            start_node_index += node_increment[DIR_X];
        }

        // 節点、要素番号増加
        start_node_index += node_increment[DIR_X];
        i += element_increment[DIR_X];
    }
    // 梁フランジ
    start_node_index =
        modeling_data->column_hexa.head.node +
        (modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[COLUMN_START_Z] + 1) * modeling_data->column_hexa.increment[2].node +
        (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + 1) * modeling_data->column_hexa.increment[DIR_Y].node;
    start_elmemnt_index = 
        modeling_data->column_hexa.head.element +
        (modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[COLUMN_START_Z]) * modeling_data->column_hexa.increment[2].element +
        (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + 1) * modeling_data->column_hexa.increment[DIR_Y].element;
    end_element_index = 
        modeling_data->column_hexa.head.element +
        (modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[COLUMN_START_Z]) * modeling_data->column_hexa.increment[2].element +
        (modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] - 1) * modeling_data->column_hexa.increment[DIR_Y].element;
    
    fprintf(f, "\n---- boundary beam element\n"); 
    for(int i = start_elmemnt_index; i <= end_element_index;) {
        // 左側
        print_HEXA_increment(f, i, start_node_index, node_increment, typh.joint_inner);
        int set = modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X];
        // 下面
        print_COPYELM(f, i, 0, 0, element_increment[DIR_X], node_increment[DIR_X], set);
        // 上面
        int elm_inc = (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1) * element_increment[2];
        int node_inc = (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1) * node_increment[2];
        print_COPYELM(f, i, i + element_increment[DIR_X] * set, element_increment[DIR_X], elm_inc, node_inc, 1);

        // 右側
        int _i_ = i + (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X] - 1) * element_increment[DIR_X];
        int _start_node_index_ = start_node_index + (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * node_increment[DIR_X];
        print_HEXA_increment(f, _i_, _start_node_index_, node_increment, typh.joint_inner);
        set = modeling_data->boundary_index[COLUMN_BEAM_X] - modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X];
        // 下面
        print_COPYELM(f, _i_, 0, 0, element_increment[DIR_X], node_increment[DIR_X], set);
        // 上面
        elm_inc = (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1) * element_increment[2];
        node_inc = (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1) * node_increment[2];
        print_COPYELM(f, _i_, _i_ + element_increment[DIR_X] * set, element_increment[DIR_X], elm_inc, node_inc, 1);

        if(i == end_element_index) {
            printf("success\n");
            break;
        }
        // 節点、要素番号増加
        start_node_index += node_increment[DIR_Y];
        i += element_increment[DIR_Y];
    }

    // 直交梁、下左
    fprintf(f, "\n---- edge 1\n");
    start_node_index = joint_origin_node +
        (modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * node_increment[DIR_X];
    start_elmemnt_index = joint_origin_elememnt + 
        (modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * element_increment[DIR_X];
    int hexa_node[8] = {
        start_node_index,
        start_node_index + node_increment[DIR_X] + node_increment[DIR_Z],
        start_node_index + node_increment[DIR_X] + node_increment[DIR_Y] + node_increment[DIR_Z],
        start_node_index + node_increment[DIR_Y],
        start_node_index + 2 * node_increment[DIR_Z],
        start_node_index + node_increment[DIR_X] + 2 * node_increment[DIR_Z],
        start_node_index + node_increment[DIR_X] + node_increment[DIR_Y] + 2 * node_increment[DIR_Z],
        start_node_index + node_increment[DIR_Y] + 2 * node_increment[DIR_Z],
    };
    print_HEXA_node(f, start_elmemnt_index, hexa_node, typh.joint_inner);
    int pre_element = start_elmemnt_index;

    // 直交梁、下右
    fprintf(f, "\n---- edge 2\n");
    start_node_index = joint_origin_node +
        (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X] - 1) * node_increment[DIR_X];
    start_elmemnt_index = joint_origin_elememnt +
        (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X] - 1) * element_increment[DIR_X];
    hexa_node[0] = start_node_index + node_increment[DIR_X] + node_increment[DIR_Z];
    hexa_node[1] = start_node_index + node_increment[DIR_X];
    hexa_node[2] = start_node_index + node_increment[DIR_X] + node_increment[DIR_Y];
    hexa_node[3] = start_node_index + node_increment[DIR_X] + node_increment[DIR_Y] + node_increment[DIR_Z];
    hexa_node[4] = start_node_index + node_increment[DIR_X] + 2 * node_increment[DIR_Z];
    hexa_node[5] = start_node_index + 2 * node_increment[DIR_X] + 2 * node_increment[DIR_Z];
    hexa_node[6] = start_node_index + 2 * node_increment[DIR_X] + node_increment[DIR_Y] + 2 * node_increment[DIR_Z];
    hexa_node[7] = start_node_index + node_increment[DIR_X] + node_increment[DIR_Y] + 2 * node_increment[DIR_Z];
    print_HEXA_node(f, start_elmemnt_index, hexa_node, typh.joint_inner);

    element_set = modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] - 1;
    print_COPYELM(f, pre_element, start_elmemnt_index, start_elmemnt_index - pre_element, element_increment[DIR_Y], node_increment[DIR_Y], element_set);

    // 直交梁、上左
    fprintf(f, "\n---- edge 3\n");
    start_node_index = joint_origin_node +
        (modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * node_increment[DIR_X] +
        (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z]) * node_increment[DIR_Z];
    start_elmemnt_index = joint_origin_elememnt +
        (modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * element_increment[DIR_X] +
        (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1) * element_increment[DIR_Z];
    hexa_node[0] = start_node_index;
    hexa_node[1] = start_node_index + node_increment[DIR_X];
    hexa_node[2] = start_node_index + node_increment[DIR_X] + node_increment[DIR_Y];
    hexa_node[3] = start_node_index + node_increment[DIR_Y];
    hexa_node[4] = start_node_index + 2 * node_increment[DIR_Z];
    hexa_node[5] = start_node_index + node_increment[DIR_X] + node_increment[DIR_Z];
    hexa_node[6] = start_node_index + node_increment[DIR_X] + node_increment[DIR_Y] + node_increment[DIR_Z];
    hexa_node[7] = start_node_index + node_increment[DIR_Y] + 2 * node_increment[DIR_Z];
    print_HEXA_node(f, start_elmemnt_index, hexa_node, typh.joint_inner);
    pre_element = start_elmemnt_index;

    // 直交梁、上右
    fprintf(f, "\n---- edge 4\n");
    start_node_index = joint_origin_node +
        (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * node_increment[DIR_X] +
        (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z]) * node_increment[DIR_Z];
    start_elmemnt_index = joint_origin_elememnt +
        (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X] - 1) * element_increment[DIR_X] +
        (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1) * element_increment[DIR_Z];
    hexa_node[0] = start_node_index;
    hexa_node[1] = start_node_index + node_increment[DIR_X];
    hexa_node[2] = start_node_index + node_increment[DIR_X] + node_increment[DIR_Y];
    hexa_node[3] = start_node_index + node_increment[DIR_Y];
    hexa_node[4] = start_node_index + node_increment[DIR_Z];
    hexa_node[5] = start_node_index + 2 * node_increment[DIR_Z];
    hexa_node[6] = start_node_index + node_increment[DIR_Y] + 2 * node_increment[DIR_Z];
    hexa_node[7] = start_node_index + node_increment[DIR_Y] + node_increment[DIR_Z];
    print_HEXA_node(f, start_elmemnt_index, hexa_node, typh.joint_inner);
    print_COPYELM(f, pre_element, start_elmemnt_index, start_elmemnt_index - pre_element, element_increment[DIR_Y], node_increment[DIR_Y], element_set);

    // 角、左下
    fprintf(f, "\n---- edge 5\n");
    start_node_index = joint_origin_node +
        (modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * node_increment[DIR_X] +
        (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y];
    start_elmemnt_index = joint_origin_elememnt +
        (modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * element_increment[DIR_X] +
        (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * element_increment[DIR_Y];
    hexa_node[0] = start_node_index;
    hexa_node[1] = start_node_index + node_increment[DIR_X] + node_increment[DIR_Z];
    hexa_node[2] = start_node_index + node_increment[DIR_X] + node_increment[DIR_Y] + node_increment[DIR_Z];
    hexa_node[3] = start_node_index + node_increment[DIR_Y] + node_increment[DIR_Z];
    hexa_node[4] = start_node_index + 2 * node_increment[DIR_Z];
    hexa_node[5] = start_node_index + node_increment[DIR_X] + 2 * node_increment[DIR_Z];
    hexa_node[6] = start_node_index + node_increment[DIR_X] + node_increment[DIR_Y] + 2 * node_increment[DIR_Z];
    hexa_node[7] = start_node_index + node_increment[DIR_Y] + 2 * node_increment[DIR_Z];
    print_HEXA_node(f, start_elmemnt_index, hexa_node, typh.joint_inner);

    // 角、右下
    fprintf(f, "\n---- edge 6\n");
    start_node_index = joint_origin_node +
        (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X] - 1) * node_increment[DIR_X] +
        (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y];
    start_elmemnt_index = joint_origin_elememnt +
        (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X] - 1) * element_increment[DIR_X] +
        (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * element_increment[DIR_Y];
    hexa_node[0] = start_node_index + node_increment[DIR_X] + node_increment[DIR_Z];
    hexa_node[1] = start_node_index + node_increment[DIR_X];
    hexa_node[2] = start_node_index + 2 * node_increment[DIR_X] + node_increment[DIR_Y] + node_increment[DIR_Z];
    hexa_node[3] = start_node_index + node_increment[DIR_X] + node_increment[DIR_Y] + node_increment[DIR_Z];
    hexa_node[4] = start_node_index + node_increment[DIR_X] + 2 * node_increment[DIR_Z];
    hexa_node[5] = start_node_index + 2 * node_increment[DIR_X] + 2 * node_increment[DIR_Z];
    hexa_node[6] = start_node_index + 2 * node_increment[DIR_X] + node_increment[DIR_Y] + 2 * node_increment[DIR_Z];
    hexa_node[7] = start_node_index + node_increment[DIR_X] + node_increment[DIR_Y] + 2 * node_increment[DIR_Z];
    print_HEXA_node(f, start_elmemnt_index, hexa_node, typh.joint_inner);

    // 角、左上
    fprintf(f, "\n---- edge 7\n");
    start_node_index = joint_origin_node +
        (modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * node_increment[DIR_X] +
        (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y] +
        (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z]) * node_increment[DIR_Z];
    start_elmemnt_index = joint_origin_elememnt +
        (modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * element_increment[DIR_X] +
        (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * element_increment[DIR_Y] +
        (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1) * element_increment[DIR_Z];
    hexa_node[0] = start_node_index;
    hexa_node[1] = start_node_index + node_increment[DIR_X];
    hexa_node[2] = start_node_index + node_increment[DIR_X] + node_increment[DIR_Y];
    hexa_node[3] = start_node_index + node_increment[DIR_Y];
    hexa_node[4] = start_node_index + 2 * node_increment[DIR_Z];
    hexa_node[5] = start_node_index + node_increment[DIR_X] + node_increment[DIR_Z];
    hexa_node[6] = start_node_index + node_increment[DIR_X] + node_increment[DIR_Y] + node_increment[DIR_Z];
    hexa_node[7] = start_node_index + node_increment[DIR_Y] + node_increment[DIR_Z];
    print_HEXA_node(f, start_elmemnt_index, hexa_node, typh.joint_inner);

    // 角、右上
    fprintf(f, "\n---- edge 8\n");
    start_node_index = joint_origin_node +
        (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * node_increment[DIR_X] +
        (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y] +
        (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z]) * node_increment[DIR_Z];
    start_elmemnt_index = joint_origin_elememnt +
        (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X] - 1) * element_increment[DIR_X] +
        (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * element_increment[DIR_Y] +
        (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1) * element_increment[DIR_Z];
    hexa_node[0] = start_node_index;
    hexa_node[1] = start_node_index + node_increment[DIR_X];
    hexa_node[2] = start_node_index + node_increment[DIR_X] + node_increment[DIR_Y];
    hexa_node[3] = start_node_index + node_increment[DIR_Y];
    hexa_node[4] = start_node_index + node_increment[DIR_Z];
    hexa_node[5] = start_node_index + 2 * node_increment[DIR_Z];
    hexa_node[6] = start_node_index + node_increment[DIR_X] + node_increment[DIR_Y] + node_increment[DIR_Z];
    hexa_node[7] = start_node_index + node_increment[DIR_Y] + node_increment[DIR_Z];
    print_HEXA_node(f, start_elmemnt_index, hexa_node, typh.joint_inner);

    // 梁、左側
    fprintf(f, "\n---- edge 9\n");
    start_node_index = joint_origin_node +
        (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y];
    start_elmemnt_index = joint_origin_elememnt +
        (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * element_increment[DIR_Y];
    hexa_node[0] = start_node_index;
    hexa_node[1] = start_node_index + node_increment[DIR_X];
    hexa_node[2] = start_node_index + node_increment[DIR_X] + node_increment[DIR_Y] + node_increment[DIR_Z];
    hexa_node[3] = start_node_index + node_increment[DIR_Y] + node_increment[DIR_Z];
    hexa_node[4] = start_node_index + 2 * node_increment[DIR_Z];
    hexa_node[5] = start_node_index + node_increment[DIR_X] + 2 * node_increment[DIR_Z] ;
    hexa_node[6] = start_node_index + node_increment[DIR_X] + node_increment[DIR_Y] + 2 * node_increment[DIR_Z];
    hexa_node[7] = start_node_index + node_increment[DIR_Y] + 2 * node_increment[DIR_Z];
    print_HEXA_node(f, start_elmemnt_index, hexa_node, typh.joint_inner);
    pre_element = start_elmemnt_index;

    start_node_index = joint_origin_node +
        (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y] +
        (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z]) * node_increment[DIR_Z];
    start_elmemnt_index = joint_origin_elememnt +
        (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * element_increment[DIR_Y] +
        (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1) * element_increment[DIR_Z];
    hexa_node[0] = start_node_index;
    hexa_node[1] = start_node_index + node_increment[DIR_X];
    hexa_node[2] = start_node_index + node_increment[DIR_X] + node_increment[DIR_Y];
    hexa_node[3] = start_node_index + node_increment[DIR_Y];
    hexa_node[4] = start_node_index + 2 * node_increment[DIR_Z];
    hexa_node[5] = start_node_index + node_increment[DIR_X] + 2 * node_increment[DIR_Z] ;
    hexa_node[6] = start_node_index + node_increment[DIR_X] + node_increment[DIR_Y] + node_increment[DIR_Z];
    hexa_node[7] = start_node_index + node_increment[DIR_Y] + node_increment[DIR_Z];
    print_HEXA_node(f, start_elmemnt_index, hexa_node, typh.joint_inner);
    element_set = modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X] - 1;
    print_COPYELM(f, pre_element, start_elmemnt_index, start_elmemnt_index - pre_element, element_increment[DIR_X], node_increment[DIR_X], element_set);

    // 梁、右側
    fprintf(f, "\n---- edge 10\n");
    start_node_index = joint_origin_node +
        (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * node_increment[DIR_X] +
        (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y];
    start_elmemnt_index = joint_origin_elememnt +
        (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * element_increment[DIR_X] +
        (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * element_increment[DIR_Y];
    hexa_node[0] = start_node_index;
    hexa_node[1] = start_node_index + node_increment[DIR_X];
    hexa_node[2] = start_node_index + 2 * node_increment[DIR_X] + node_increment[DIR_Y] + node_increment[DIR_Z];
    hexa_node[3] = start_node_index + node_increment[DIR_X] + node_increment[DIR_Y] + node_increment[DIR_Z];
    hexa_node[4] = start_node_index + node_increment[DIR_X] + 2 * node_increment[DIR_Z];
    hexa_node[5] = start_node_index + 2 * node_increment[DIR_X] + 2 * node_increment[DIR_Z];
    hexa_node[6] = start_node_index + 2 * node_increment[DIR_X] + node_increment[DIR_Y] + 2 * node_increment[DIR_Z];
    hexa_node[7] = start_node_index + node_increment[DIR_X] + node_increment[DIR_Y] + 2 * node_increment[DIR_Z];
    print_HEXA_node(f, start_elmemnt_index, hexa_node, typh.joint_inner);
    pre_element = start_elmemnt_index;

    start_node_index = joint_origin_node +
        (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 1) * node_increment[DIR_X] +
        (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y] +
        (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z]) * node_increment[DIR_Z];
    start_elmemnt_index = joint_origin_elememnt +
        (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * element_increment[DIR_X] +
        (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * element_increment[DIR_Y] +
        (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1) * element_increment[DIR_Z];
    hexa_node[0] = start_node_index;
    hexa_node[1] = start_node_index + node_increment[DIR_X];
    hexa_node[2] = start_node_index + node_increment[DIR_X] + node_increment[DIR_Y];
    hexa_node[3] = start_node_index + node_increment[DIR_Y];
    hexa_node[4] = start_node_index - node_increment[DIR_X] + 2 * node_increment[DIR_Z];
    hexa_node[5] = start_node_index + 2 * node_increment[DIR_Z] ;
    hexa_node[6] = start_node_index + node_increment[DIR_X] + node_increment[DIR_Y] + node_increment[DIR_Z];
    hexa_node[7] = start_node_index + node_increment[DIR_Y] + node_increment[DIR_Z];
    print_HEXA_node(f, start_elmemnt_index, hexa_node, typh.joint_inner);
    element_set = modeling_data->boundary_index[COLUMN_BEAM_X] - modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - 1;
    print_COPYELM(f, pre_element, start_elmemnt_index, start_elmemnt_index - pre_element, element_increment[DIR_X], node_increment[DIR_X], element_set);

    // 外部要素、右
    fprintf(f, "\n---- edge 11\n");
    node_increment[DIR_Z] = 2 * modeling_data->column_hexa.increment[2].node;

    start_node_index = joint_origin_node;
    start_elmemnt_index = joint_origin_elememnt;
    print_HEXA_increment(f, start_elmemnt_index, start_node_index, node_increment, typh.joint_inner);
    pre_element = start_elmemnt_index;

    start_node_index = joint_origin_node +
        (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z]) * modeling_data->column_hexa.increment[2].node;
    start_elmemnt_index = joint_origin_elememnt + 
        (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1) * element_increment[DIR_Z];
    print_HEXA_increment(f, start_elmemnt_index, start_node_index, node_increment, typh.joint_inner);
    element_set = modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X] - 1;
    print_COPYELM(f, pre_element, start_elmemnt_index, start_elmemnt_index - pre_element, element_increment[DIR_X], node_increment[DIR_X], element_set);
    int element_diff = element_set;
    element_set = modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] - 1;
    print_COPYELM(f, pre_element, pre_element + element_diff, element_increment[DIR_X], element_increment[DIR_Y], node_increment[DIR_Y], element_set);
    print_COPYELM(f, start_elmemnt_index, start_elmemnt_index + element_diff, element_increment[DIR_X], element_increment[DIR_Y], node_increment[DIR_Y], element_set);

    // 外部要素、右
    node_increment[DIR_Z] = modeling_data->column_hexa.increment[0].node + 2 * modeling_data->column_hexa.increment[2].node;
    start_node_index = joint_origin_node +
        (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * node_increment[DIR_X];
    start_elmemnt_index = joint_origin_elememnt +
        (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * element_increment[DIR_X];
    print_HEXA_increment(f, start_elmemnt_index, start_node_index, node_increment, typh.joint_inner);
    pre_element = start_elmemnt_index;

    node_increment[DIR_Z] = - modeling_data->column_hexa.increment[0].node + 2 * modeling_data->column_hexa.increment[2].node;
    start_node_index = joint_origin_node +
        (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 1) * node_increment[DIR_X] +
        (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z]) * modeling_data->column_hexa.increment[2].node;
    start_elmemnt_index = joint_origin_elememnt +
        (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * element_increment[DIR_X] +
        (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1) * element_increment[DIR_Z];
    print_HEXA_increment(f, start_elmemnt_index, start_node_index, node_increment, typh.joint_inner);
    element_set = modeling_data->boundary_index[COLUMN_BEAM_X] - modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - 1;
    print_COPYELM(f, pre_element, start_elmemnt_index, start_elmemnt_index - pre_element, element_increment[DIR_X], node_increment[DIR_X], element_set);
    element_diff = element_set;
    element_set = modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] - 1;
    print_COPYELM(f, pre_element, pre_element + element_diff, element_increment[DIR_X], element_increment[DIR_Y], node_increment[DIR_Y], element_set);
    print_COPYELM(f, start_elmemnt_index, start_elmemnt_index + element_diff, element_increment[DIR_X], element_increment[DIR_Y], node_increment[DIR_Y], element_set);
    fprintf(f, "\n");

    // 接合部外部要素、要素番号
    // 左
    for(int i = modeling_data->boundary_index[COLUMN_SURFACE_START_Y]; i < modeling_data->boundary_index[COLUMN_BEAM_Y]; i++) {
        start_elmemnt_index =
            modeling_data->column_hexa.head.element + (modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[COLUMN_START_Z]) * element_increment[DIR_Z] +
            i * element_increment[DIR_Y];
        end_element_index =
            start_elmemnt_index + (modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X] - 1) * element_increment[DIR_X];
        element_set = (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1);
        print_ETYP(f, start_elmemnt_index, end_element_index, element_increment[DIR_X], typh.joint_outer, element_increment[DIR_Z], element_set);
    }
    // 右
    for(int i = modeling_data->boundary_index[COLUMN_SURFACE_START_Y]; i < modeling_data->boundary_index[COLUMN_BEAM_Y]; i++) {
        start_elmemnt_index =
            modeling_data->column_hexa.head.element + (modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[COLUMN_START_Z]) * element_increment[DIR_Z] +
            (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * element_increment[DIR_X] +
            i * element_increment[DIR_Y];
        end_element_index =
            start_elmemnt_index + (modeling_data->boundary_index[COLUMN_BEAM_X] - modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - 1) * element_increment[DIR_X];
        element_set = (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1);
        print_ETYP(f, start_elmemnt_index, end_element_index, element_increment[DIR_X], typh.joint_outer, element_increment[DIR_Z], element_set);
    }
    fprintf(f, "\n");
}

/**
 * 柱の節点番号を返す.
 * x,y,zは0から始まる
 */
int search_column_node(int column_head, int increment[], int boundary_index[], int x, int y, int z) {
    
    if(x < 0 || y < 0 || z < 0) {
        printf("error ## < 0\n");
        return -1;  // 不正な値
    }

    int x_form_column = x - boundary_index[BEAM_COLUMN_X];

    // z方向、柱、接合部上下面、接合部内か判定
    if(z <= boundary_index[COLUMN_BEAM_Z]) {
        // 下柱
        return column_head + increment[DIR_X] * x_form_column  + increment[DIR_Y] * y  + increment[DIR_Z] * z;
    } else if(z <= boundary_index[BEAM_COLUMN_Z] + 1) {
        // 接合部
        if(x <= boundary_index[COLUMN_CENTER_X]) {
            return column_head + increment[DIR_X] * x_form_column  + increment[DIR_Y] * y  + increment[DIR_Z] * z;
        } else {
            return column_head + increment[DIR_X] * (x_form_column + 1) + increment[DIR_Y] * y + increment[DIR_Z] * z;
        }
    }else if(z <= boundary_index[COLUMN_END_Z] + 2) {
        // 上柱
        return column_head + increment[DIR_X] * x_form_column  + increment[DIR_Y] * y  + increment[DIR_Z] * z;
    } else {
        printf("error\n");
        return -1;
    }
    return -1;
}

/**
 * 柱主筋
 */
void add_reber_fiber_line(FILE *f, ModelingData *modeling_data) {
    fprintf(f, "---- REBAR FIBER LINE ----\n");
    // ポインタ配列に各方向を格納
    NodeCoordinate* coordinates[3] = {modeling_data->x, modeling_data->y, modeling_data->z};

    int node_increment[3] = {
        0,
        0,
        modeling_data->rebar_fiber->increment.node
    };

    
    for(int i = 0; i < modeling_data->rebar_fiber->rebar_num; i++) {
        fprintf(f, "---- fiber\n");
        int start[3] = {
            modeling_data->rebar_fiber->positions[i].x,
            modeling_data->rebar_fiber->positions[i].y,
            modeling_data->boundary_index[JIG_COLUMN_Z]
        };
        int end[3] = {
            modeling_data->rebar_fiber->positions[i].x,
            modeling_data->rebar_fiber->positions[i].y,
            modeling_data->boundary_index[COLUMN_JIG_Z]
        };
        int start_node = modeling_data->rebar_fiber->head.node + i * modeling_data->rebar_fiber->occupied_indices_single.node;
        int start_element = modeling_data->rebar_fiber->head.element + i * modeling_data->rebar_fiber->occupied_indices_single.element;
        // 最初の主筋の定義
        plot_node(f, coordinates, start_node, start, end, node_increment);
        print_BEAM(f, start_element, start_node, modeling_data->rebar_fiber->increment.node, 1);
        int element_set = (modeling_data->boundary_index[COLUMN_JIG_Z] - modeling_data->boundary_index[JIG_COLUMN_Z] - 1);
        print_COPYELM(f, start_element, 0, 0, modeling_data->rebar_fiber->increment.element, modeling_data->rebar_fiber->increment.node, element_set);
        fprintf(f, "\n");
        // ライン要素
        fprintf(f, "---- line\n");
        // 下柱部分
        int column_increment[3] = {
            modeling_data->column_hexa.increment[DIR_X].node,
            modeling_data->column_hexa.increment[DIR_Y].node,
            modeling_data->column_hexa.increment[DIR_Z].node
        };
        int line_start_element = modeling_data->rebar_line.head.element + i * modeling_data->rebar_line.occupied_indices_single.element;
        int column_node = search_column_node(modeling_data->column_hexa.head.node, column_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y], start[DIR_Z]);
        print_LINE_increment(f, line_start_element, start_node, column_node, modeling_data->rebar_fiber->increment.node);
        element_set = (modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[JIG_COLUMN_Z] - 1);
        if(element_set > 0) {
            print_COPYELM(f, line_start_element, 0, 0, modeling_data->rebar_line.increment.element, modeling_data->rebar_fiber->increment.node, element_set);
        }
        // 接合部下境界面
        start[DIR_Z] = modeling_data->boundary_index[COLUMN_BEAM_Z];
        end[DIR_Z] = modeling_data->boundary_index[COLUMN_BEAM_Z] + 2;
        line_start_element =
            modeling_data->rebar_line.head.element +
            (modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[JIG_COLUMN_Z]) * modeling_data->rebar_line.increment.element +
            i * modeling_data->rebar_line.occupied_indices_single.element;
        int line_node[4] = {
            modeling_data->rebar_fiber->head.node +
                (modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[JIG_COLUMN_Z]) * modeling_data->rebar_fiber->increment.node +
                i * modeling_data->rebar_fiber->occupied_indices_single.node,
            modeling_data->rebar_fiber->head.node +
                (modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[JIG_COLUMN_Z] + 1) * modeling_data->rebar_fiber->increment.node +
                i * modeling_data->rebar_fiber->occupied_indices_single.node,
            search_column_node(modeling_data->column_hexa.head.node, column_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y], start[DIR_Z]),
            search_column_node(modeling_data->column_hexa.head.node, column_increment, modeling_data->boundary_index, end[DIR_X], end[DIR_Y], end[DIR_Z])
        };
        print_LINE_node(f, line_start_element, line_node);
        // 接合部内
        if(modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] > 2 ) {
            start[DIR_Z] = modeling_data->boundary_index[COLUMN_BEAM_Z] + 2;
            end[DIR_Z] = modeling_data->boundary_index[COLUMN_BEAM_Z] + 3;
            line_start_element =
                modeling_data->rebar_line.head.element +
                (modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[JIG_COLUMN_Z] + 1) * modeling_data->rebar_line.increment.element +
                i * modeling_data->rebar_line.occupied_indices_single.element;
            line_node[0] = modeling_data->rebar_fiber->head.node +
                (modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[JIG_COLUMN_Z] + 1) * modeling_data->rebar_fiber->increment.node +
                i * modeling_data->rebar_fiber->occupied_indices_single.node;
            line_node[1] = modeling_data->rebar_fiber->head.node +
                (modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[JIG_COLUMN_Z] + 2) * modeling_data->rebar_fiber->increment.node +
                i * modeling_data->rebar_fiber->occupied_indices_single.node,
            line_node[2] = search_column_node(modeling_data->column_hexa.head.node, column_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y], start[DIR_Z]);
            line_node[3] = search_column_node(modeling_data->column_hexa.head.node, column_increment, modeling_data->boundary_index, end[DIR_X], end[DIR_Y], end[DIR_Z]);
            print_LINE_node(f, line_start_element, line_node);
            element_set = (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 3);
            if(element_set > 0) {
                print_COPYELM(f, line_start_element, 0, 0, modeling_data->rebar_fiber->increment.element, modeling_data->rebar_fiber->increment.node, element_set);
            }
        }
        // 接合部上境界面
        start[DIR_Z] = modeling_data->boundary_index[BEAM_COLUMN_Z];
        end[DIR_Z] = modeling_data->boundary_index[BEAM_COLUMN_Z] + 2;
        line_start_element =
            modeling_data->rebar_line.head.element +
            (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[JIG_COLUMN_Z] - 1) * modeling_data->rebar_line.increment.element +
            i * modeling_data->rebar_line.occupied_indices_single.element;
        line_node[0] = modeling_data->rebar_fiber->head.node +
            (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[JIG_COLUMN_Z] - 1) * modeling_data->rebar_fiber->increment.node +
            i * modeling_data->rebar_fiber->occupied_indices_single.node;
        line_node[1] = modeling_data->rebar_fiber->head.node +
            (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[JIG_COLUMN_Z]) * modeling_data->rebar_fiber->increment.node +
            i * modeling_data->rebar_fiber->occupied_indices_single.node,
        line_node[2] = search_column_node(modeling_data->column_hexa.head.node, column_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y], start[DIR_Z]);
        line_node[3] = search_column_node(modeling_data->column_hexa.head.node, column_increment, modeling_data->boundary_index, end[DIR_X], end[DIR_Y], end[DIR_Z]);
        print_LINE_node(f, line_start_element, line_node);
        // 上柱
        start[DIR_Z] = modeling_data->boundary_index[BEAM_COLUMN_Z] + 2;
        end[DIR_Z] = modeling_data->boundary_index[BEAM_COLUMN_Z] + 3;
        line_start_element =
            modeling_data->rebar_line.head.element +
            (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[JIG_COLUMN_Z]) * modeling_data->rebar_line.increment.element +
            i * modeling_data->rebar_line.occupied_indices_single.element;
        line_node[0] = modeling_data->rebar_fiber->head.node +
            (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[JIG_COLUMN_Z]) * modeling_data->rebar_fiber->increment.node +
            i * modeling_data->rebar_fiber->occupied_indices_single.node;
        line_node[1] = modeling_data->rebar_fiber->head.node +
            (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[JIG_COLUMN_Z] + 1) * modeling_data->rebar_fiber->increment.node +
            i * modeling_data->rebar_fiber->occupied_indices_single.node,
        line_node[2] = search_column_node(modeling_data->column_hexa.head.node, column_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y], start[DIR_Z]);
        line_node[3] = search_column_node(modeling_data->column_hexa.head.node, column_increment, modeling_data->boundary_index, end[DIR_X], end[DIR_Y], end[DIR_Z]);
        print_LINE_node(f, line_start_element, line_node);
        element_set = (modeling_data->boundary_index[COLUMN_JIG_Z] - modeling_data->boundary_index[BEAM_COLUMN_Z] - 1);
        if(element_set > 0) {
            print_COPYELM(f, line_start_element, 0, 0, modeling_data->rebar_fiber->increment.element, modeling_data->rebar_fiber->increment.node, element_set);
        }
        fprintf(f, "\n");
    }
    fprintf(f, "\n");
}

/**
 * 接合部四辺形要素
 */
void add_joint_quad(FILE *f, ModelingData *modeling_data) {
    fprintf(f, "---- JOINT QUAD ----\n");
    int typq[3] = {
        8,
        5,
        10
    };

    // ポインタ配列に各方向を格納
    NodeCoordinate* coordinates[3] = {modeling_data->x, modeling_data->y, modeling_data->z};

    int node_increment[3] = {
        modeling_data->column_hexa.increment[DIR_X].node,
        modeling_data->column_hexa.increment[DIR_Y].node,
        modeling_data->column_hexa.increment[DIR_Z].node
    };
    int element_increment[3] = {
        modeling_data->joint_quad.increment_element[DIR_X],
        modeling_data->joint_quad.increment_element[DIR_Y],
        modeling_data->joint_quad.increment_element[DIR_Z]
    };

    
    // yz面 支圧板、ふさぎ板、直交梁ウェブ ---------------------------------------------------------
    fprintf(f, "---- yz\n");
    for(int i = 0; i < 3; i++) {
        BoundaryType boundary_type;
        switch (i) {
        case 0:
            // ふさぎ板
            boundary_type = BEAM_COLUMN_X;
            break;
        case 1:
            // 直交梁ウェブ
            boundary_type = COLUMN_CENTER_X;
            break;
        case 2:
            // ふさぎ板
            boundary_type = COLUMN_BEAM_X;
            break;
        default:
            // 例外
            boundary_type = -1;
            break;
        }
        int start_node =
            modeling_data->joint_quad.head.node +
            (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[BEAM_COLUMN_X]) * node_increment[DIR_X];
        int start[3] = {
            modeling_data->boundary_index[boundary_type],
            modeling_data->boundary_index[COLUMN_SURFACE_START_Y],
            modeling_data->boundary_index[COLUMN_BEAM_Z]
        };
        int end[3] = {
            modeling_data->boundary_index[boundary_type],
            modeling_data->boundary_index[CENTER_Y],
            modeling_data->boundary_index[BEAM_COLUMN_Z]
        };
        // 節点定義
        plot_node(f, coordinates, start_node, start, end, node_increment);
        int start_element =
            modeling_data->joint_quad.head.element +
            (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[BEAM_COLUMN_X] + i) * element_increment[DIR_X] +
            element_increment[DIR_Y] +
            element_increment[DIR_Z];
        print_QUAD_increment(f, start_element, start_node, node_increment, DIR_Y, DIR_Z, typq[i]);
        print_COPYELM(f, start_element, 0, 0, element_increment[DIR_Y], node_increment[DIR_Y], modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] - 1);
        print_COPYELM(
            f,
            start_element,
            start_element + (modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] - 1) * element_increment[DIR_Y],
            element_increment[DIR_Y],
            element_increment[DIR_Z],
            node_increment[DIR_Z],
            modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1
        );
        fprintf(f, "\n");
    }

    // xz面 支圧板、ふさぎ板、直交梁ウェブ ---------------------------------------------------------
    fprintf(f, "---- zx\n");
    int typq_[2]= {
        9,
        2
    };
    for(int i = 0; i < 2; i++) {
        BoundaryType boundary_type;
        switch (i) {
        case 0:
            // 手前ふさぎ板
            boundary_type = COLUMN_SURFACE_START_Y;
            break;
        case 1:
            // 梁ウェブ
            boundary_type = CENTER_Y;
            break;
        default:
            boundary_type = -1;
            break;
        }
        int start_node =
            modeling_data->joint_quad.head.node +
            node_increment[DIR_X] +
            (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y];
        int start[3] = {
            modeling_data->boundary_index[BEAM_COLUMN_X] + 1,
            modeling_data->boundary_index[boundary_type],
            modeling_data->boundary_index[COLUMN_BEAM_Z]
        };
        int end[3] = {
            modeling_data->boundary_index[COLUMN_CENTER_X] - 1,
            modeling_data->boundary_index[boundary_type],
            modeling_data->boundary_index[BEAM_COLUMN_Z]
        };
        // 節点定義 - 左
        plot_node(f, coordinates, start_node, start, end, node_increment);
        start_node =
            modeling_data->joint_quad.head.node +
            (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 1) * node_increment[DIR_X] +
            (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y];
        start[DIR_X] = modeling_data->boundary_index[COLUMN_CENTER_X] + 1;
        end[DIR_X] = modeling_data->boundary_index[COLUMN_BEAM_X] - 1;
        // 節点定義 - 右
        plot_node(f, coordinates, start_node, start, end, node_increment);
        start_node =
            modeling_data->joint_quad.head.node +
            (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y];
        int start_element =
            modeling_data->joint_quad.head.element +
            element_increment[DIR_X] +
            (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + i) * element_increment[DIR_Y] +
            element_increment[DIR_Z];
        print_QUAD_increment(f, start_element, start_node, node_increment, DIR_Z, DIR_X, typq_[i]);
        print_COPYELM(f, start_element, 0, 0, element_increment[DIR_X], node_increment[DIR_X], modeling_data->boundary_index[COLUMN_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X] - 1);
        print_COPYELM(
            f,
            start_element,
            start_element + (modeling_data->boundary_index[COLUMN_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X] - 1) * element_increment[DIR_X],
            element_increment[DIR_X],
            element_increment[DIR_Z],
            node_increment[DIR_Z],
            modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1
        );
        fprintf(f, "\n");
    }
    
    // xy平面 フランジ ---------------------------------------------------------
    fprintf(f, "---- xy\n");
    int cros_typq_[2] = {
        7,
        6
    };
    int _typq_[2] = {
        4,
        3
    };
    for(int i = 0; i < 2; i++) {
        BoundaryType z_boundary = COLUMN_BEAM_Z;
        switch (i) {
        case 0:
            // 下
            z_boundary = COLUMN_BEAM_Z;
            break;
        case 1:
            // 上
            z_boundary = BEAM_COLUMN_Z;
            break;
        default:
            // 例外
            break;
        }

        int start[3] = {
            modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X],
            modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + 1,
            modeling_data->boundary_index[z_boundary]
        };
        int end[3] = {
            modeling_data->boundary_index[COLUMN_CENTER_X] - 1,
            modeling_data->boundary_index[COLUMN_BEAM_Y] - 1,
            modeling_data->boundary_index[z_boundary]
        };
        int start_node =
            modeling_data->joint_quad.head.node +
            (modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * node_increment[DIR_X] +
            node_increment[DIR_Y] +
            (modeling_data->boundary_index[z_boundary] - modeling_data->boundary_index[COLUMN_BEAM_Z]) * node_increment[DIR_Z];

        // 節点定義 - 直交フランジ
        plot_node(f, coordinates, start_node, start, end, node_increment);
        start_node =
            modeling_data->joint_quad.head.node +
            (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 1) * node_increment[DIR_X] +
            node_increment[DIR_Y] +
            (modeling_data->boundary_index[z_boundary] - modeling_data->boundary_index[COLUMN_BEAM_Z]) * node_increment[DIR_Z];
        start[0] = modeling_data->boundary_index[COLUMN_CENTER_X] + 1;
        end[0] = modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X];
        plot_node(f, coordinates, start_node, start, end, node_increment);

        // 節点定義 - 梁フランジ
        start_node =
            modeling_data->joint_quad.head.node +
            node_increment[DIR_X] +
            (modeling_data->boundary_index[COLUMN_BEAM_Y]) *node_increment[DIR_Y] +
            (modeling_data->boundary_index[z_boundary] - modeling_data->boundary_index[COLUMN_BEAM_Z]) * node_increment[DIR_Z];
        start[0] = modeling_data->boundary_index[BEAM_COLUMN_X] + 1;
        start[1] = modeling_data->boundary_index[COLUMN_BEAM_Y];
        end[0] = modeling_data->boundary_index[COLUMN_CENTER_X] - 1;
        end[1] = modeling_data->boundary_index[CENTER_Y] - 1;
        plot_node(f, coordinates, start_node, start, end, node_increment);
        start_node =
            modeling_data->joint_quad.head.node +
            (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 1) * node_increment[DIR_X] +
            (modeling_data->boundary_index[COLUMN_BEAM_Y]) *node_increment[DIR_Y] +
            (modeling_data->boundary_index[z_boundary] - modeling_data->boundary_index[COLUMN_BEAM_Z]) * node_increment[DIR_Z];
        start[0] = modeling_data->boundary_index[COLUMN_CENTER_X] + 1;
        start[1] = modeling_data->boundary_index[COLUMN_BEAM_Y];
        end[0] = modeling_data->boundary_index[COLUMN_BEAM_X] - 1;
        end[1] = modeling_data->boundary_index[CENTER_Y] - 1;
        plot_node(f, coordinates, start_node, start, end, node_increment);

        // 要素定義 - 直交梁フランジ
        start_node =
            modeling_data->joint_quad.head.node +
            (modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * node_increment[DIR_X] +
            (modeling_data->boundary_index[z_boundary] - modeling_data->boundary_index[COLUMN_BEAM_Z]) * node_increment[DIR_Z];
        int start_element =
            modeling_data->joint_quad.head.element +
            (modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 1) * element_increment[DIR_X] +
            element_increment[DIR_Y] +
            i * (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] + 1) * element_increment[DIR_Z];
        int node[4] = {
            start_node,
            start_node + node_increment[DIR_X],
            start_node + node_increment[DIR_X] + node_increment[DIR_Y],
            start_node + node_increment[DIR_Y]
        };
        print_QUAD_node(f, start_element, node, cros_typq_[i]);
        print_COPYELM(f, start_element, 0, 0, element_increment[0], node_increment[DIR_X], (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - 1));
        print_COPYELM(
            f,
            start_element,
            start_element + (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - 1) * element_increment[0],
            element_increment[0],
            element_increment[DIR_Y],
            node_increment[DIR_Y],
            modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] - 1
        );
        start_node =
            modeling_data->joint_quad.head.node +
            (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * node_increment[DIR_X] +
            (modeling_data->boundary_index[z_boundary] - modeling_data->boundary_index[COLUMN_BEAM_Z]) * node_increment[DIR_Z];
        start_element =
            modeling_data->joint_quad.head.element +
            (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 2) * element_increment[DIR_X] +
            element_increment[DIR_Y] +
            i * (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] + 1) * element_increment[DIR_Z];
        node[0] = start_node;
        node[1] = start_node + node_increment[DIR_X];
        node[2] = start_node + node_increment[DIR_X] + node_increment[DIR_Y];
        node[3] = start_node + node_increment[DIR_Y];
        print_QUAD_node(f, start_element, node, cros_typq_[i]);
        print_COPYELM(f, start_element, 0, 0, element_increment[0], node_increment[DIR_X], (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[COLUMN_CENTER_X] - 1));
        print_COPYELM(
            f,
            start_element,
            start_element + (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[COLUMN_CENTER_X] - 1) * element_increment[0],
            element_increment[0],
            element_increment[DIR_Y],
            node_increment[DIR_Y],
            modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] - 1
        );
        // 要素定義 - 梁フランジ
        start_node =
            modeling_data->joint_quad.head.node +
            (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y] +
            (modeling_data->boundary_index[z_boundary] - modeling_data->boundary_index[COLUMN_BEAM_Z]) * node_increment[DIR_Z];
        start_element =
            modeling_data->joint_quad.head.element +
            element_increment[DIR_X] +
            (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + 1) * element_increment[DIR_Y] +
            (modeling_data->boundary_index[z_boundary] - modeling_data->boundary_index[COLUMN_BEAM_Z] + i) * element_increment[DIR_Z];
        node[0] = start_node;
        node[1] = start_node + node_increment[DIR_X];
        node[2] = start_node + node_increment[DIR_X] + node_increment[DIR_Y];
        node[3] = start_node + node_increment[DIR_Y];
        print_QUAD_node(f, start_element, node, _typq_[i]);
        print_COPYELM(f, start_element, 0, 0, element_increment[0], node_increment[DIR_X], (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X] - 1));
        print_COPYELM(
            f,
            start_element,
            start_element + (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X] - 1) * element_increment[0],
            element_increment[0],
            element_increment[DIR_Y],
            node_increment[DIR_Y],
            modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_BEAM_Y] - 1
        );
        start_node =
            modeling_data->joint_quad.head.node +
            (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * node_increment[DIR_X] +
            (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y] +
            (modeling_data->boundary_index[z_boundary] - modeling_data->boundary_index[COLUMN_BEAM_Z]) * node_increment[DIR_Z];
        start_element =
            modeling_data->joint_quad.head.element +
            (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 2) * element_increment[DIR_X] +
            (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + 1) * element_increment[DIR_Y] +
            (modeling_data->boundary_index[z_boundary] - modeling_data->boundary_index[COLUMN_BEAM_Z] + i) * element_increment[DIR_Z];
        node[0] = start_node;
        node[1] = start_node + node_increment[DIR_X];
        node[2] = start_node + node_increment[DIR_X] + node_increment[DIR_Y];
        node[3] = start_node + node_increment[DIR_Y];
        print_QUAD_node(f, start_element, node, _typq_[i]);
        print_COPYELM(f, start_element, 0, 0, element_increment[0], node_increment[DIR_X], (modeling_data->boundary_index[COLUMN_BEAM_X] - modeling_data->boundary_index[COLUMN_CENTER_X] - 1));
        print_COPYELM(
            f,
            start_element,
            start_element + (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X] - 1) * element_increment[0],
            element_increment[0],
            element_increment[DIR_Y],
            node_increment[DIR_Y],
            modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_BEAM_Y] - 1
        );
        fprintf(f, "\n");
    }

    fprintf(f, "\n");
}

/**
 * 接合部FILM要素
 */
void add_joint_film(FILE *f, ModelingData *modeling_data) {
    fprintf(f, "---- JOINT FILM ----\n");

    int node_increment[3] = {
        modeling_data->column_hexa.increment[DIR_X].node,
        modeling_data->column_hexa.increment[DIR_Y].node,
        modeling_data->column_hexa.increment[DIR_Z].node
    };
    int element_increment[3] = {
        modeling_data->joint_quad.increment_element[DIR_X],
        modeling_data->joint_quad.increment_element[DIR_Y],
        modeling_data->joint_quad.increment_element[DIR_Z]
    };

    // yz面 支圧板、ふさぎ板、直交梁ウェブ ---------------------------------------------------------
    fprintf(f, "---- yz\n");
    for(int i = 0; i < 3; i++) {
        BoundaryType boundary_type;
        switch (i) {
        case 0:
            // ふさぎ板
            boundary_type = BEAM_COLUMN_X;
            break;
        case 1:
            // 直交梁ウェブ
            boundary_type = COLUMN_CENTER_X;
            break;
        case 2:
            // ふさぎ板
            boundary_type = COLUMN_BEAM_X;
            break;
        default:
            // 例外
            boundary_type = -1;
            break;
        }
        int start_node =
            modeling_data->joint_quad.head.node +
            (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[BEAM_COLUMN_X]) * node_increment[DIR_X];
        int start[3] = {
            modeling_data->boundary_index[boundary_type],
            modeling_data->boundary_index[COLUMN_SURFACE_START_Y],
            modeling_data->boundary_index[COLUMN_BEAM_Z]
        };

        if(boundary_type == BEAM_COLUMN_X || boundary_type == COLUMN_BEAM_X) {
            int typf = 1;
            int element_diff = 0;
            if(boundary_type == COLUMN_BEAM_X) {
                element_diff = 3;
            }
            // 外部要素
            // 柱付け根（必須）
            int film_index =
                modeling_data->joint_film.head +
                (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[BEAM_COLUMN_X] + element_diff) * element_increment[DIR_X] +
                element_increment[DIR_Y] +
                2 * element_increment[DIR_Z];
            int face1[4] = {
                start_node,
                start_node + node_increment[DIR_Y],
                start_node + node_increment[DIR_Z] + node_increment[DIR_Y],
                start_node + node_increment[DIR_Z]
            };
            int face2[4] = {
                search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y], start[DIR_Z]),
                search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y] + 1, start[DIR_Z]),
                search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y] + 1, start[DIR_Z] + 2),
                search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y], start[DIR_Z] + 2)
            };
            if(boundary_type == COLUMN_BEAM_X) {
                // 配列の要素順番を逆転して局所座標系のz軸方向を逆にする
                reverse_array(face1, 4);
                reverse_array(face2, 4);
            }
            print_FILM_node(f, film_index, face1, face2, typf);
            int pre_element = film_index;
            film_index += (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1) * element_increment[DIR_Z];
            for(int j = 0; j < 4; j++) {
                face1[j] += (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1) * node_increment[DIR_Z];
            }
            start[DIR_Z] = modeling_data->boundary_index[BEAM_COLUMN_Z];
            face2[0] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y], start[DIR_Z]);
            face2[1] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y] + 1, start[DIR_Z]);
            face2[2] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y] + 1, start[DIR_Z] + 2);
            face2[3] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y], start[DIR_Z] + 2);
            if(boundary_type == COLUMN_BEAM_X) {
                // 配列の要素順番を逆転して局所座標系のz軸方向を逆にする
                reverse_array(face2, 4);
            }
            print_FILM_node(f, film_index, face1, face2, typf);
            print_COPYELM(f, pre_element, film_index, film_index - pre_element, element_increment[DIR_Y], node_increment[DIR_Y], modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] - 1);
            // 境界 必ずあると仮定
            start[DIR_Y] = modeling_data->boundary_index[COLUMN_BEAM_Y];
            start[DIR_Z] = modeling_data->boundary_index[COLUMN_BEAM_Z];
            film_index =
                modeling_data->joint_film.head +
                (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[BEAM_COLUMN_X] + element_diff) * element_increment[DIR_X] +
                (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + 1) * element_increment[DIR_Y] +
                2 * element_increment[DIR_Z];
            face1[0] = start_node + (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y];
            face1[1] = face1[0] + node_increment[DIR_Y];
            face1[2] = face1[0] + node_increment[DIR_Z] + node_increment[DIR_Y];
            face1[3] = face1[0] + node_increment[DIR_Z];
            face2[0] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y], start[DIR_Z]);
            face2[1] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y] + 1, start[DIR_Z] + 1);
            face2[2] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y] + 1, start[DIR_Z] + 2);
            face2[3] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y], start[DIR_Z] + 2);
            if(boundary_type == COLUMN_BEAM_X) {
                // 配列の要素順番を逆転して局所座標系のz軸方向を逆にする
                reverse_array(face1, 4);
                reverse_array(face2, 4);
            }
            print_FILM_node(f, film_index, face1, face2, typf);
            film_index += (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1) * element_increment[DIR_Z];
            for(int j = 0; j < 4; j++) {
                face1[j] += (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1) * node_increment[DIR_Z];
            }
            start[DIR_Z] = modeling_data->boundary_index[BEAM_COLUMN_Z];
            face2[0] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y], start[DIR_Z]);
            face2[1] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y] + 1, start[DIR_Z]);
            face2[2] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y] + 1, start[DIR_Z] + 1);
            face2[3] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y], start[DIR_Z] + 2);
            if(boundary_type == COLUMN_BEAM_X) {
                // 配列の要素順番を逆転して局所座標系のz軸方向を逆にする
                reverse_array(face2, 4);
            }
            print_FILM_node(f, film_index, face1, face2, typf);

            int element_set = modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_BEAM_Y] - 1;
            if(element_set > 0) {
                start[DIR_Y] = modeling_data->boundary_index[COLUMN_BEAM_Y] + 1;
                start[DIR_Z] = modeling_data->boundary_index[COLUMN_BEAM_Z] + 1;

                film_index =
                    modeling_data->joint_film.head +
                    (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[BEAM_COLUMN_X] + element_diff) * element_increment[DIR_X] +
                    (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + 2) * element_increment[DIR_Y] +
                    2 * element_increment[DIR_Z];
                face1[0] = start_node + (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + 1) * node_increment[DIR_Y];
                face1[1] = face1[0] + node_increment[DIR_Y];
                face1[2] = face1[0] + node_increment[DIR_Z] + node_increment[DIR_Y];
                face1[3] = face1[0] + node_increment[DIR_Z];
                face2[0] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y], start[DIR_Z]);
                face2[1] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y] + 1, start[DIR_Z]);
                face2[2] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y] + 1, start[DIR_Z] + 1);
                face2[3] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y], start[DIR_Z] + 1);
                if(boundary_type == COLUMN_BEAM_X) {
                    // 配列の要素順番を逆転して局所座標系のz軸方向を逆にする
                    reverse_array(face1, 4);
                    reverse_array(face2, 4);
                }
                print_FILM_node(f, film_index, face1, face2, typf);
                int z_diff = modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1;
                print_COPYELM(f, film_index, 0, 0, z_diff * element_increment[DIR_Z], z_diff * node_increment[DIR_Z], 1);
                if(element_set > 1) {
                    print_COPYELM(f, film_index, film_index + (z_diff * element_increment[DIR_Z]), z_diff * element_increment[DIR_Z], element_increment[DIR_Y], node_increment[DIR_Y], element_set - 1);
                }
            }
            // 中央
            if(modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] > 2) {
                start[DIR_Y] = modeling_data->boundary_index[COLUMN_SURFACE_START_Y];
                start[DIR_Z] = modeling_data->boundary_index[COLUMN_BEAM_Z] + 2;
                film_index =
                    modeling_data->joint_film.head +
                    (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[BEAM_COLUMN_X] + element_diff) * element_increment[DIR_X] +
                    element_increment[DIR_Y] +
                    3 * element_increment[DIR_Z];
                face1[0] = start_node + node_increment[DIR_Z];
                face1[1] = face1[0] + node_increment[DIR_Y];
                face1[2] = face1[0] + node_increment[DIR_Z] + node_increment[DIR_Y];
                face1[3] = face1[0] + node_increment[DIR_Z];
                face2[0] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y], start[DIR_Z]);
                face2[1] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y] + 1, start[DIR_Z]);
                face2[2] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y] + 1, start[DIR_Z] + 1);
                face2[3] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y], start[DIR_Z] + 1);
                if(boundary_type == COLUMN_BEAM_X) {
                    // 配列の要素順番を逆転して局所座標系のz軸方向を逆にする
                    reverse_array(face1, 4);
                    reverse_array(face2, 4);
                }
                print_FILM_node(f, film_index, face1, face2, typf);
                print_COPYELM(f, film_index, 0, 0, element_increment[DIR_Y], node_increment[DIR_Y], modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] - 1);
                print_COPYELM(f, film_index, film_index + element_increment[DIR_Y] * (modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] - 1), element_increment[DIR_Y], element_increment[DIR_Z], node_increment[DIR_Z], modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 3);
            }
        }
        // 直交梁ウェブ
        if(boundary_type == COLUMN_CENTER_X) {
            start[DIR_Y] = modeling_data->boundary_index[COLUMN_SURFACE_START_Y];
            start[DIR_Z] = modeling_data->boundary_index[COLUMN_BEAM_Z] + 1;

            int film_index =
                modeling_data->joint_film.head +
                (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 1) * element_increment[DIR_X] +
                element_increment[DIR_Y] +
                2 * element_increment[DIR_Z];
            int face1[4] = {
                start_node,
                start_node + node_increment[DIR_Z],
                start_node + node_increment[DIR_Z] + node_increment[DIR_Y],
                start_node + node_increment[DIR_Y]
            };
            int face2[4] = {
                search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y], start[DIR_Z]),
                search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y], start[DIR_Z] + 1),
                search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y] + 1, start[DIR_Z] + 1),
                search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y] + 1, start[DIR_Z])
            };
            print_FILM_node(f, film_index, face1, face2, 2);
            film_index += element_increment[DIR_X];
            for(int j = 0; j < 4; j++) {
                face2[j] += modeling_data->column_hexa.increment[DIR_X].node;
            }
            // 配列の要素順番を逆転して局所座標系のz軸方向を逆にする
            reverse_array(face1, 4);
            reverse_array(face2, 4);
            print_FILM_node(f, film_index, face1, face2, 2);
            print_COPYELM(f, film_index - element_increment[DIR_X], film_index, element_increment[DIR_X], element_increment[DIR_Y], node_increment[DIR_Y], modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] - 1);
            print_COPYELM(f, film_index - element_increment[DIR_X], film_index - element_increment[DIR_X] + element_increment[DIR_Y] * (modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] - 1), element_increment[DIR_Y], element_increment[DIR_Z], node_increment[DIR_Z], modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1);
            print_COPYELM(f, film_index, film_index + element_increment[DIR_Y] * (modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] - 1), element_increment[DIR_Y], element_increment[DIR_Z], node_increment[DIR_Z], modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1);
        }
    }

    // xz面 支圧板、ふさぎ板、直交梁ウェブ ---------------------------------------------------------
    fprintf(f, "---- zx\n");
    for(int i = 0; i < 2; i++) {
        /**
         * 左 -> 右
         */
        BoundaryType boundary_type;
        switch (i) {
        case 0:
            // 手前ふさぎ板
            boundary_type = COLUMN_SURFACE_START_Y;
            break;
        case 1:
            // ウェブ
            boundary_type = CENTER_Y;
            break;
        default:
            boundary_type = -1;
            break;
        }
        int start_node =
            modeling_data->joint_quad.head.node +
            node_increment[DIR_X] +
            (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y];
        int start[3] = {
            modeling_data->boundary_index[BEAM_COLUMN_X] + 1,
            modeling_data->boundary_index[boundary_type],
            modeling_data->boundary_index[COLUMN_BEAM_Z]
        };
        start_node =
            modeling_data->joint_quad.head.node +
            (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 1) * node_increment[DIR_X] +
            (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y];
        start[DIR_X] = modeling_data->boundary_index[COLUMN_CENTER_X] + 1;
        start_node =
            modeling_data->joint_quad.head.node +
            (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y];
        int start_element =
            modeling_data->joint_quad.head.element +
            element_increment[DIR_X] +
            (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + i) * element_increment[DIR_Y] +
            element_increment[DIR_Z];
        // 手前ふさぎ板
        if(boundary_type == COLUMN_SURFACE_START_Y) {
            int element_diff = 0;
            // フル対応？
            if(boundary_type == COLUMN_SURFACE_END_Y) {
                element_diff = 3;
            }
            // 左
            // 外部要素
            fprintf(f, "---- out\n");
            start[DIR_X] = modeling_data->boundary_index[BEAM_COLUMN_X];
            start_element =
                modeling_data->joint_film.head +
                element_increment[DIR_X] +
                (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + element_diff) * element_increment[DIR_Y] +
                2 * element_increment[DIR_Z];
            start_node =
                modeling_data->joint_quad.head.node +
                (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y];
            int face1[4] = {
                start_node,
                start_node + node_increment[DIR_Z],
                start_node + node_increment[DIR_Z] + node_increment[DIR_X],
                start_node + node_increment[DIR_X]
            };
            int face2[4] = {
                search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y], start[DIR_Z]),
                search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y], start[DIR_Z] + 2),
                search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X] + 1, start[DIR_Y], start[DIR_Z] + 2),
                search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X] + 1, start[DIR_Y], start[DIR_Z])
            };
            print_FILM_node(f, start_element, face1, face2, 1);
            int pre_element = start_element;
            start[DIR_Z] = modeling_data->boundary_index[BEAM_COLUMN_Z];
            start_element =
                modeling_data->joint_film.head +
                element_increment[DIR_X] +
                (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + element_diff) * element_increment[DIR_Y] +
                (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] + 1) * element_increment[DIR_Z];
            start_node =
                modeling_data->joint_quad.head.node +
                (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y] +
                (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1) * node_increment[DIR_Z];
            face1[0] = start_node;
            face1[1] = face1[0] + node_increment[DIR_Z];
            face1[2] = face1[0] + node_increment[DIR_X] + node_increment[DIR_Z];
            face1[3] = face1[0] + node_increment[DIR_X];
            face2[0] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y], start[DIR_Z]);
            face2[1] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y], start[DIR_Z] + 2);
            face2[2] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X] + 1, start[DIR_Y], start[DIR_Z] + 2);
            face2[3] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X] + 1, start[DIR_Y], start[DIR_Z]);
            print_FILM_node(f, start_element, face1, face2, 1);
            print_COPYELM(f, pre_element, start_element, start_element - pre_element, element_increment[DIR_X], node_increment[DIR_X], modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X] - 1);
            // 境界
            fprintf(f, "---- bou\n");
            start[DIR_X] = modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X];
            start[DIR_Z] = modeling_data->boundary_index[COLUMN_BEAM_Z];
            start_element =
                modeling_data->joint_film.head +
                (modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 1) * element_increment[DIR_X] +
                (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + element_diff) * element_increment[DIR_Y] +
                2 * element_increment[DIR_Z];
            start_node =
                modeling_data->joint_quad.head.node +
                (modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * node_increment[DIR_X] +
                (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y];
            face1[0] = start_node;
            face1[1] = face1[0] + node_increment[DIR_Z];
            face1[2] = face1[0] + node_increment[DIR_X] + node_increment[DIR_Z];
            face1[3] = face1[0] + node_increment[DIR_X];
            face2[0] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y], start[DIR_Z]);
            face2[1] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y], start[DIR_Z] + 2);
            face2[2] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X] + 1, start[DIR_Y], start[DIR_Z] + 2);
            face2[3] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X] + 1, start[DIR_Y], start[DIR_Z] + 1);
            print_FILM_node(f, start_element, face1, face2, 1);
            start[DIR_Z] = modeling_data->boundary_index[BEAM_COLUMN_Z];
            start_element +=
                (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1) * element_increment[DIR_Z];
            start_node =
                modeling_data->joint_quad.head.node +
                (modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * node_increment[DIR_X] +
                (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y] +
                (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1) * node_increment[DIR_Z];
            face1[0] = start_node;
            face1[1] = face1[0] + node_increment[DIR_Z];
            face1[2] = face1[0] + node_increment[DIR_X] + node_increment[DIR_Z];
            face1[3] = face1[0] + node_increment[DIR_X];
            face2[0] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y], start[DIR_Z]);
            face2[1] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y], start[DIR_Z] + 2);
            face2[2] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X] + 1, start[DIR_Y], start[DIR_Z] + 1);
            face2[3] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X] + 1, start[DIR_Y], start[DIR_Z]);
            print_FILM_node(f, start_element, face1, face2, 1);
            // 内部
            fprintf(f, "---- inner\n");
            int element_set = modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - 1;
            if(element_set > 0) {
                start[DIR_X] = modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] + 1;
                start[DIR_Z] = modeling_data->boundary_index[COLUMN_BEAM_Z] + 1;
                start_element =
                    modeling_data->joint_film.head +
                    (modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 2) * element_increment[DIR_X] +
                    (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + element_diff) * element_increment[DIR_Y] +
                    2 * element_increment[DIR_Z];
                start_node =
                    modeling_data->joint_quad.head.node +
                    (modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 1) * node_increment[DIR_X] +
                    (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y];
                face1[0] = start_node;
                face1[1] = face1[0] + node_increment[DIR_Z];
                face1[2] = face1[0] + node_increment[DIR_X] + node_increment[DIR_Z];
                face1[3] = face1[0] + node_increment[DIR_X];
                face2[0] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y], start[DIR_Z]);
                face2[1] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y], start[DIR_Z] + 1);
                face2[2] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X] + 1, start[DIR_Y], start[DIR_Z] + 1);
                face2[3] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X] + 1, start[DIR_Y], start[DIR_Z]);
                print_FILM_node(f, start_element, face1, face2, 1);
                print_COPYELM(f, start_element, 0, 0, (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1) * element_increment[DIR_Z], (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1) * node_increment[DIR_Z], 1);
                if(element_set > 1) {
                    print_COPYELM(
                        f,
                        start_element,
                        start_element + (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1) * element_increment[DIR_Z],
                        (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1) * element_increment[DIR_Z],
                        element_increment[DIR_X],
                        node_increment[DIR_X],
                        element_set - 1
                    );
                }
            }
            // 上下端を除いた要素
            fprintf(f, "---- center\n");
            if(modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] > 2) {
                start[DIR_X] = modeling_data->boundary_index[BEAM_COLUMN_X];
                start[DIR_Z] = modeling_data->boundary_index[COLUMN_BEAM_Z] + 2;
                start_element =
                    modeling_data->joint_film.head +
                    element_increment[DIR_X] +
                    (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + element_diff) * element_increment[DIR_Y] +
                    3 * element_increment[DIR_Z];
                start_node =
                    modeling_data->joint_quad.head.node +
                    (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y] +
                    node_increment[DIR_Z];
                face1[0] = start_node;
                face1[1] = face1[0] + node_increment[DIR_Z];
                face1[2] = face1[0] + node_increment[DIR_X] + node_increment[DIR_Z];
                face1[3] = face1[0] + node_increment[DIR_X];
                face2[0] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y], start[DIR_Z]);
                face2[1] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X], start[DIR_Y], start[DIR_Z] + 1);
                face2[2] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X] + 1, start[DIR_Y], start[DIR_Z] + 1);
                face2[3] = search_column_node(modeling_data->column_hexa.head.node, node_increment, modeling_data->boundary_index, start[DIR_X] + 1, start[DIR_Y], start[DIR_Z]);
                print_FILM_node(f, start_element, face1, face2, 1);
                print_COPYELM(f, start_element, 0, 0, element_increment[DIR_Z], node_increment[DIR_Z], modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 3);
                print_COPYELM(
                    f,
                    start_element,
                    start_element + (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 3) * element_increment[DIR_Z],
                    element_increment[DIR_Z],
                    element_increment[DIR_X],
                    node_increment[DIR_X],
                    modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X] - 1
                );
                // 内部
                element_set = modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[COLUMN_CENTER_X] - 1;
                if(element_set > 0) {
                    start_element =
                        modeling_data->joint_film.head +
                        (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 3) * element_increment[DIR_X] +
                        (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + element_diff) * element_increment[DIR_Y] +
                        2 * element_increment[DIR_Z];
                    start_node =
                        modeling_data->joint_quad.head.node +
                        (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * node_increment[DIR_X] +
                        (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y];
                    int column_node =
                        modeling_data->column_hexa.head.node +
                        (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 1) * node_increment[DIR_X] +
                        (modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[COLUMN_START_Z] + 1) * node_increment[DIR_Z];
                    face1[0] = start_node;
                    face1[1] = face1[0] + node_increment[DIR_Z];
                    face1[2] = face1[0] + node_increment[DIR_X] + node_increment[DIR_Z];
                    face1[3] = face1[0] + node_increment[DIR_X];
                    face2[0] = column_node;
                    face2[1] = column_node + node_increment[DIR_Z];
                    face2[2] = column_node + node_increment[DIR_X] + node_increment[DIR_Z];
                    face2[3] = column_node + node_increment[DIR_X];
                    print_FILM_node(f, start_element, face1, face2, 1);
                    print_COPYELM(f, start_element, 0, 0, (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1) * element_increment[DIR_Z], (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1) * node_increment[DIR_Z], 1);
                    if(element_set > 1) {
                        print_COPYELM(
                            f,
                            start_element,
                            start_element + (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1) * element_increment[DIR_Z],
                            (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1) * element_increment[DIR_Z],
                            element_increment[DIR_X],
                            node_increment[DIR_X],
                            element_set - 1
                        );
                    }
                }
            }
            // 右
            // 境界
            fprintf(f, "---- bou\n");
            start_element =
                modeling_data->joint_film.head +
                (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 2) * element_increment[DIR_X] +
                (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + element_diff) * element_increment[DIR_Y] +
                2 * element_increment[DIR_Z];
            start_node =
                modeling_data->joint_quad.head.node +
                (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X] - 1) * node_increment[DIR_X] +
                (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y];
            int column_node =
                modeling_data->column_hexa.head.node +
                (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * node_increment[DIR_X] +
                (modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[COLUMN_START_Z] + 1) * node_increment[DIR_Z];
            face1[0] = start_node;
            face1[1] = face1[0] + node_increment[DIR_Z];
            face1[2] = face1[0] + node_increment[DIR_X] + node_increment[DIR_Z];
            face1[3] = face1[0] + node_increment[DIR_X];
            face2[0] = column_node;
            face2[1] = column_node + node_increment[DIR_Z];
            face2[2] = column_node + node_increment[DIR_X] + node_increment[DIR_Z];
            face2[3] = column_node - node_increment[DIR_Z];
            print_FILM_node(f, start_element, face1, face2, 1);
            start_element =
                modeling_data->joint_film.head +
                (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 2) * element_increment[DIR_X] +
                (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + element_diff) * element_increment[DIR_Y] +
                (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] + 1) * element_increment[DIR_Z];
            start_node =
                modeling_data->joint_quad.head.node +
                (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X] - 1) * node_increment[DIR_X] +
                (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y] +
                (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1) * node_increment[DIR_Z];
            column_node =
                modeling_data->column_hexa.head.node +
                (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * node_increment[DIR_X] +
                (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_START_Z]) * node_increment[DIR_Z];
            face1[0] = start_node;
            face1[1] = face1[0] + node_increment[DIR_Z];
            face1[2] = face1[0] + node_increment[DIR_X] + node_increment[DIR_Z];
            face1[3] = face1[0] + node_increment[DIR_X];
            face2[0] = column_node;
            face2[1] = column_node + node_increment[DIR_Z];
            face2[2] = column_node + 2 * node_increment[DIR_Z];
            face2[3] = column_node + node_increment[DIR_X];
            print_FILM_node(f, start_element, face1, face2, 1);
            // 外部
            fprintf(f, "---- out\n");
            start_element =
                modeling_data->joint_film.head +
                (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 3) * element_increment[DIR_X] +
                (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + element_diff) * element_increment[DIR_Y] +
                2 * element_increment[DIR_Z];
            start_node =
                modeling_data->joint_quad.head.node +
                (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * node_increment[DIR_X] +
                (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y];
            column_node =
                modeling_data->column_hexa.head.node +
                (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * node_increment[DIR_X] +
                (modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[COLUMN_START_Z]) * node_increment[DIR_Z];
            face1[0] = start_node;
            face1[1] = face1[0] + node_increment[DIR_Z];
            face1[2] = face1[0] + node_increment[DIR_X] + node_increment[DIR_Z];
            face1[3] = face1[0] + node_increment[DIR_X];
            face2[0] = column_node;
            face2[1] = column_node + node_increment[DIR_X] + 2 * node_increment[DIR_Z];
            face2[2] = column_node + 2 * node_increment[DIR_X] + 2 *node_increment[DIR_Z];
            face2[3] = column_node + node_increment[DIR_X];
            print_FILM_node(f, start_element, face1, face2, 1);
            pre_element = start_element;
            start_element =
                modeling_data->joint_film.head +
                (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 3) * element_increment[DIR_X] +
                (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + element_diff) * element_increment[DIR_Y] +
                (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] + 1) * element_increment[DIR_Z];
            start_node =
                modeling_data->joint_quad.head.node +
                (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * node_increment[DIR_X] +
                (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y] +
                (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1) * node_increment[DIR_Z];
            column_node =
                modeling_data->column_hexa.head.node +
                (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 1) * node_increment[DIR_X] +
                (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_START_Z]) * node_increment[DIR_Z];
            face1[0] = start_node;
            face1[1] = face1[0] + node_increment[DIR_Z];
            face1[2] = face1[0] + node_increment[DIR_X] + node_increment[DIR_Z];
            face1[3] = face1[0] + node_increment[DIR_X];
            face2[0] = column_node;
            face2[1] = column_node - node_increment[DIR_X] + 2 * node_increment[DIR_Z];
            face2[2] = column_node + 2 *node_increment[DIR_Z];
            face2[3] = column_node + node_increment[DIR_X];
            print_FILM_node(f, start_element, face1, face2, 1);
            print_COPYELM(f, pre_element, start_element, start_element - pre_element, element_increment[DIR_X], node_increment[DIR_X], modeling_data->boundary_index[COLUMN_BEAM_X] - modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - 1);
            
            // 中央
            fprintf(f, "---- center\n");
            if(modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] > 2) {
                start_element =
                    modeling_data->joint_film.head +
                    (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 3) * element_increment[DIR_X] +
                    (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + element_diff) * element_increment[DIR_Y] +
                    3 * element_increment[DIR_Z];
                start_node =
                    modeling_data->joint_quad.head.node +
                    (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * node_increment[DIR_X] +
                    (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y] +
                    node_increment[DIR_Z];
                column_node =
                    modeling_data->column_hexa.head.node +
                    (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 1) * node_increment[DIR_X] +
                    (modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[COLUMN_START_Z] + 2) * node_increment[DIR_Z];
                face1[0] = start_node;
                face1[1] = face1[0] + node_increment[DIR_Z];
                face1[2] = face1[0] + node_increment[DIR_X] + node_increment[DIR_Z];
                face1[3] = face1[0] + node_increment[DIR_X];
                face2[0] = column_node;
                face2[1] = column_node + node_increment[DIR_Z];
                face2[2] = column_node + node_increment[DIR_X] + node_increment[DIR_Z];
                face2[3] = column_node + node_increment[DIR_X];
                print_FILM_node(f, start_element, face1, face2, 1);
                print_COPYELM(f, start_element, 0, 0, element_increment[DIR_Z], node_increment[DIR_Z], modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 3);
                print_COPYELM(
                    f,
                    start_element,
                    start_element + (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 3) * element_increment[DIR_Z],
                    element_increment[DIR_Z],
                    element_increment[DIR_X],
                    node_increment[DIR_X],
                    modeling_data->boundary_index[COLUMN_BEAM_X] - modeling_data->boundary_index[COLUMN_CENTER_X] - 1
                );
            }
        }
        
        if(boundary_type == CENTER_Y) {
            // ウェブ
            start_element =
                modeling_data->joint_film.head +
                element_increment[DIR_X] +
                (modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + 1) * element_increment[DIR_Y] +
                2 * element_increment[DIR_Z];
            start_node =
                modeling_data->joint_quad.head.node +
                (modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y];
            int column_node =
                modeling_data->column_hexa.head.node +
                (modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y] +
                (modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[COLUMN_START_Z] + 1) * node_increment[DIR_Z];
            int face1[4] = {
                start_node,
                start_node + node_increment[DIR_X],
                start_node + node_increment[DIR_X] + node_increment[DIR_Z],
                start_node + node_increment[DIR_Z]
            };
            int face2[4] = {
                column_node,
                column_node + node_increment[DIR_X],
                column_node + node_increment[DIR_X] + node_increment[DIR_Z],
                column_node + node_increment[DIR_Z]
            };
            print_FILM_node(f, start_element, face1, face2, 1);
            print_COPYELM(f, start_element, 0, 0, element_increment[DIR_Z], node_increment[DIR_Z], modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1);
            print_COPYELM(
                f,
                start_element,
                start_element + (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1) * element_increment[DIR_Z],
                element_increment[DIR_Z],
                element_increment[DIR_X],
                node_increment[DIR_X],
                modeling_data->boundary_index[COLUMN_BEAM_X] - modeling_data->boundary_index[COLUMN_CENTER_X] - 1
            );
            start_element =
                modeling_data->joint_film.head +
                (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 3) * element_increment[DIR_X] +
                (modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + 1) * element_increment[DIR_Y] +
                2 * element_increment[DIR_Z];
            start_node =
                modeling_data->joint_quad.head.node +
                (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * node_increment[DIR_X] +
                (modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y];
            column_node =
                modeling_data->column_hexa.head.node +
                (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 1) * node_increment[DIR_X] +
                (modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y] +
                (modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[COLUMN_START_Z] + 1) * node_increment[DIR_Z];
            face1[0] = start_node;
            face1[1] = start_node + node_increment[DIR_X];
            face1[2] = start_node + node_increment[DIR_X] + node_increment[DIR_Z];
            face1[3] = start_node + node_increment[DIR_Z];
            face2[0] = column_node;
            face2[1] = column_node + node_increment[DIR_X];
            face2[2] = column_node + node_increment[DIR_X] + node_increment[DIR_Z];
            face2[3] = column_node + node_increment[DIR_Z];
            print_FILM_node(f, start_element, face1, face2, 1);
            print_COPYELM(f, start_element, 0, 0, element_increment[DIR_Z], node_increment[DIR_Z], modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1);
            print_COPYELM(
                f,
                start_element,
                start_element + (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1) * element_increment[DIR_Z],
                element_increment[DIR_Z],
                element_increment[DIR_X],
                node_increment[DIR_X],
                modeling_data->boundary_index[COLUMN_BEAM_X] - modeling_data->boundary_index[COLUMN_CENTER_X] - 1
            );
        }
    }

    // xy平面 ---------------------------------------------------------
    fprintf(f, "---- xy\n");
    // 上下フランジ、柱面
    for(int i = 0; i < 2; i++) {
        BoundaryType boundary_type = COLUMN_BEAM_Z;
        if(i == 1) {
            boundary_type = BEAM_COLUMN_Z;
        }
        int quad_node =
            modeling_data->joint_quad.head.node + 
            (modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * node_increment[DIR_X] +
            (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[COLUMN_BEAM_Z]) * node_increment[DIR_Z];
        int hexa_node =
            modeling_data->column_hexa.head.node +
            (modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * node_increment[DIR_X] +
            (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[COLUMN_START_Z] + i * 2) * node_increment[DIR_Z];
        int film_element =
            modeling_data->joint_film.head +
            (modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 1) * element_increment[DIR_X] +
            element_increment[DIR_Y] +
            (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[COLUMN_BEAM_Z] + i * 3) * element_increment[DIR_Z];
        int face1[4] = {
            quad_node,
            quad_node + node_increment[DIR_Y],
            quad_node + node_increment[DIR_X] + node_increment[DIR_Y],
            quad_node + node_increment[DIR_X]
        };
        int face2[4] = {
            hexa_node,
            hexa_node + node_increment[DIR_Y],
            hexa_node + node_increment[DIR_X] + node_increment[DIR_Y],
            hexa_node + node_increment[DIR_X]
        };
        if(boundary_type == BEAM_COLUMN_Z) {
            // 配列の要素順番を逆転して局所座標系のz軸方向を逆にする
            reverse_array(face1, 4);
            reverse_array(face2, 4);
        }
        print_FILM_node(f, film_element, face1, face2, 1);
        print_COPYELM(f, film_element, 0, 0, element_increment[DIR_X], node_increment[DIR_X], modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - 1);
        print_COPYELM(
            f,
            film_element,
            film_element + (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - 1) * element_increment[DIR_X],
            element_increment[DIR_X],
            element_increment[DIR_Y],
            node_increment[DIR_Y],
            modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] - 1
        );
        quad_node += (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X]) * node_increment[DIR_X];
        hexa_node += (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X]) * node_increment[DIR_X];
        film_element += (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] + 1) * node_increment[DIR_X];
        face1[0] = quad_node;
        face1[1] = quad_node + node_increment[DIR_Y];
        face1[2] = quad_node + node_increment[DIR_X] + node_increment[DIR_Y];
        face1[3] = quad_node + node_increment[DIR_X];
        face2[0] = hexa_node;
        face2[1] = hexa_node + node_increment[DIR_Y];
        face2[2] = hexa_node + node_increment[DIR_X] + node_increment[DIR_Y];
        face2[3] = hexa_node + node_increment[DIR_X];
        if(boundary_type == BEAM_COLUMN_Z) {
            // 配列の要素順番を逆転して局所座標系のz軸方向を逆にする
            reverse_array(face1, 4);
            reverse_array(face2, 4);
        }
        print_FILM_node(f, film_element, face1, face2, 1);
        print_COPYELM(f, film_element, 0, 0, element_increment[DIR_X], node_increment[DIR_X], modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[COLUMN_CENTER_X] - 1);
        print_COPYELM(
            f,
            film_element,
            film_element + (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[COLUMN_CENTER_X] - 1) * element_increment[DIR_X],
            element_increment[DIR_X],
            element_increment[DIR_Y],
            node_increment[DIR_Y],
            modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] - 1
        );
        // 梁フランジ方向
        quad_node =
            modeling_data->joint_quad.head.node + 
            (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y] +
            (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[COLUMN_BEAM_Z]) * node_increment[DIR_Z];
        hexa_node =
            modeling_data->column_hexa.head.node +
            (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y] +
            (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[COLUMN_START_Z] + i * 2) * node_increment[DIR_Z];
        film_element =
            modeling_data->joint_film.head +
            (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + 1) * element_increment[DIR_Y] +
            element_increment[DIR_X] +
            (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[COLUMN_BEAM_Z] + i * 3) * element_increment[DIR_Z];
        face1[0] = quad_node;
        face1[1] = quad_node + node_increment[DIR_Y];
        face1[2] = quad_node + node_increment[DIR_X] + node_increment[DIR_Y];
        face1[3] = quad_node + node_increment[DIR_X];
        face2[0] = hexa_node;
        face2[1] = hexa_node + node_increment[DIR_Y];
        face2[2] = hexa_node + node_increment[DIR_X] + node_increment[DIR_Y];
        face2[3] = hexa_node + node_increment[DIR_X];
        if(boundary_type == BEAM_COLUMN_Z) {
            // 配列の要素順番を逆転して局所座標系のz軸方向を逆にする
            reverse_array(face1, 4);
            reverse_array(face2, 4);
        }
        print_FILM_node(f, film_element, face1, face2, 1);
        print_COPYELM(f, film_element, 0, 0, element_increment[DIR_X], node_increment[DIR_X], modeling_data->boundary_index[COLUMN_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X] - 1);
        print_COPYELM(
            f,
            film_element,
            film_element + (modeling_data->boundary_index[COLUMN_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X] - 1) * element_increment[DIR_X],
            element_increment[DIR_X],
            element_increment[DIR_Y],
            node_increment[DIR_Y],
            modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_BEAM_Y] - 1
        );
    }

    // 上下フランジ接合部面 - 境界
    // 左
    int quad_node =
        modeling_data->joint_quad.head.node + 
        (modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * node_increment[DIR_X];
    int hexa_node =
        modeling_data->column_hexa.head.node +
        (modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * node_increment[DIR_X] +
        (modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[COLUMN_START_Z]) * node_increment[DIR_Z];
    int film_element =
        modeling_data->joint_film.head +
        (modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 1) * element_increment[DIR_X] +
        element_increment[DIR_Y] +
        element_increment[DIR_Z];
    int face1[4] = {
        quad_node,
        quad_node + node_increment[DIR_X],
        quad_node + node_increment[DIR_X] + node_increment[DIR_Y],
        quad_node + node_increment[DIR_Y]
    };
    int face2[4] = {
        hexa_node,
        hexa_node + node_increment[DIR_X] + node_increment[DIR_Z],
        hexa_node + node_increment[DIR_X] + node_increment[DIR_Y] + node_increment[DIR_Z],
        hexa_node + node_increment[DIR_Y]
    };
    print_FILM_node(f, film_element, face1, face2, 1);
    int pre_element = film_element;
    // 角
    quad_node +=
        (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y];
    hexa_node +=
        (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y];
    film_element +=
        (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * element_increment[DIR_Y];
    face1[0] = quad_node;
    face1[1] = quad_node + node_increment[DIR_X];
    face1[2] = quad_node + node_increment[DIR_X] + node_increment[DIR_Y];
    face1[3] = quad_node + node_increment[DIR_Y];
    face2[0] = hexa_node;
    face2[1] = hexa_node + node_increment[DIR_X] + node_increment[DIR_Z];
    face2[2] = hexa_node + node_increment[DIR_X] + node_increment[DIR_Y] + node_increment[DIR_Z];
    face2[3] = hexa_node + node_increment[DIR_Y] + node_increment[DIR_Z];
    print_FILM_node(f, film_element, face1, face2, 1);
    // 左上
    quad_node =
        modeling_data->joint_quad.head.node + 
        (modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * node_increment[DIR_X] +
        (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z]) * node_increment[DIR_Z];
    hexa_node =
        modeling_data->column_hexa.head.node +
        (modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * node_increment[DIR_X] +
        (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_START_Z] + 2) * node_increment[DIR_Z];
    film_element =
        modeling_data->joint_film.head +
        (modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 1) * element_increment[DIR_X] +
        element_increment[DIR_Y] +
        (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] + 1) * element_increment[DIR_Z];
    face1[0] = quad_node;
    face1[1] = quad_node + node_increment[DIR_Y];
    face1[2] = quad_node + node_increment[DIR_X] + node_increment[DIR_Y];
    face1[3] = quad_node + node_increment[DIR_X];
    face2[0] = hexa_node;
    face2[1] = hexa_node + node_increment[DIR_Y];
    face2[2] = hexa_node + node_increment[DIR_X] + node_increment[DIR_Y] - node_increment[DIR_Z];
    face2[3] = hexa_node + node_increment[DIR_X] - node_increment[DIR_Z];
    print_FILM_node(f, film_element, face1, face2, 1);
    print_COPYELM(
        f,
        pre_element,
        film_element,
        film_element - pre_element,
        element_increment[DIR_Y],
        node_increment[DIR_Y],
        modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] - 1
    );
    // 角
    quad_node +=
        (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y];
    hexa_node +=
        (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y];
    film_element +=
        (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * element_increment[DIR_Y];
    face1[0] = quad_node;
    face1[1] = quad_node + node_increment[DIR_Y];
    face1[2] = quad_node + node_increment[DIR_X] + node_increment[DIR_Y];
    face1[3] = quad_node + node_increment[DIR_X];
    face2[0] = hexa_node;
    face2[1] = hexa_node + node_increment[DIR_Y] - node_increment[DIR_Z];
    face2[2] = hexa_node + node_increment[DIR_X] + node_increment[DIR_Y] - node_increment[DIR_Z];
    face2[3] = hexa_node + node_increment[DIR_X] - node_increment[DIR_Z];
    print_FILM_node(f, film_element, face1, face2, 1);
    
    quad_node =
        modeling_data->joint_quad.head.node + 
        (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y];
    hexa_node =
        modeling_data->column_hexa.head.node +
        (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y] +
        (modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[COLUMN_START_Z]) * node_increment[DIR_Z];
    film_element =
        modeling_data->joint_film.head +
        element_increment[DIR_X] +
        (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + 1) * element_increment[DIR_Y] +
        element_increment[DIR_Z];
    face1[0] = quad_node;
    face1[1] = quad_node + node_increment[DIR_X];
    face1[2] = quad_node + node_increment[DIR_X] + node_increment[DIR_Y];
    face1[3] = quad_node + node_increment[DIR_Y];
    face2[0] = hexa_node;
    face2[1] = hexa_node + node_increment[DIR_X];
    face2[2] = hexa_node + node_increment[DIR_X] + node_increment[DIR_Y] + node_increment[DIR_Z];
    face2[3] = hexa_node + node_increment[DIR_Y] + node_increment[DIR_Z];
    print_FILM_node(f, film_element, face1, face2, 1);
    pre_element = film_element;
    quad_node +=
        (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z]) * node_increment[DIR_Z];
    hexa_node +=
        (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] + 2) * node_increment[DIR_Z];
    film_element +=
        (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] + 1) * element_increment[DIR_Z];
    face1[0] = quad_node;
    face1[1] = quad_node + node_increment[DIR_Y];
    face1[2] = quad_node + node_increment[DIR_X] + node_increment[DIR_Y];
    face1[3] = quad_node + node_increment[DIR_X];
    face2[0] = hexa_node;
    face2[1] = hexa_node + node_increment[DIR_Y] - node_increment[DIR_Z];
    face2[2] = hexa_node + node_increment[DIR_X] + node_increment[DIR_Y] - node_increment[DIR_Z];
    face2[3] = hexa_node + node_increment[DIR_X];
    print_FILM_node(f, film_element, face1, face2, 1);
    print_COPYELM(
        f,
        pre_element,
        film_element,
        film_element - pre_element,
        element_increment[DIR_X],
        node_increment[DIR_X],
        modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X] - 1
    );
    // 内部 - 下
    int element_set = modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - 1;
    if(element_set > 0) {
        quad_node =
            modeling_data->joint_quad.head.node +
            (modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 1) * node_increment[DIR_X];
        hexa_node =
            modeling_data->column_hexa.head.node +
            (modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 1) * node_increment[DIR_X] +
            (modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[COLUMN_START_Z] + 1) * node_increment[DIR_Z];
        film_element =
            modeling_data->joint_film.head +
            (modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 2) * element_increment[DIR_X] +
            element_increment[DIR_Y] +
            element_increment[DIR_Z];
        face1[0] = quad_node;
        face1[1] = quad_node + node_increment[DIR_X];
        face1[2] = quad_node + node_increment[DIR_X] + node_increment[DIR_Y];
        face1[3] = quad_node + node_increment[DIR_Y];
        face2[0] = hexa_node;
        face2[1] = hexa_node + node_increment[DIR_X];
        face2[2] = hexa_node + node_increment[DIR_X] + node_increment[DIR_Y];
        face2[3] = hexa_node + node_increment[DIR_Y];
        print_FILM_node(f, film_element, face1, face2, 1);
        print_COPYELM(
            f,
            film_element,
            0,
            0,
            element_increment[DIR_Y],
            node_increment[DIR_Y],
            modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]
        );
        if(element_set > 1) {
            print_COPYELM(
                f,
                film_element,
                film_element + (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * element_increment[DIR_Y],
                element_increment[DIR_Y],
                element_increment[DIR_X],
                node_increment[DIR_X],
                element_set - 1
            );
        }
        // 内部 - 上
        quad_node =
            modeling_data->joint_quad.head.node +
            (modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 1) * node_increment[DIR_X] +
            (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z]) * node_increment[DIR_Z];
        hexa_node =
            modeling_data->column_hexa.head.node +
            (modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 1) * node_increment[DIR_X] +
            (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_START_Z] + 1) * node_increment[DIR_Z];
        film_element =
            modeling_data->joint_film.head +
            (modeling_data->boundary_index[COLUMN_ORTHOGONAL_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 2) * element_increment[DIR_X] +
            element_increment[DIR_Y] +
            (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] + 2) * element_increment[DIR_Z];
        face1[0] = quad_node;
        face1[1] = quad_node + node_increment[DIR_Y];
        face1[2] = quad_node + node_increment[DIR_X] + node_increment[DIR_Y];
        face1[3] = quad_node + node_increment[DIR_X];
        face2[0] = hexa_node;
        face2[1] = hexa_node + node_increment[DIR_Y];
        face2[2] = hexa_node + node_increment[DIR_X] + node_increment[DIR_Y];
        face2[3] = hexa_node + node_increment[DIR_X];

        print_FILM_node(f, film_element, face1, face2, 1);
        print_COPYELM(
            f,
            film_element,
            0,
            0,
            element_increment[DIR_Y],
            node_increment[DIR_Y],
            modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]
        );
        if(element_set > 1) {
            print_COPYELM(
                f,
                film_element,
                film_element + (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * element_increment[DIR_Y],
                element_increment[DIR_Y],
                element_increment[DIR_X],
                node_increment[DIR_X],
                element_set - 1
            );
        }
    }
    element_set = modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_BEAM_Y] - 1;
    if(element_set > 0) {
        // 梁ウェブ - 下
        quad_node =
            modeling_data->joint_quad.head.node +
            (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + 1) * node_increment[DIR_Y];
        hexa_node =
            modeling_data->column_hexa.head.node +
            (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + 1) * node_increment[DIR_Y] +
            (modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[COLUMN_START_Z] + 1) * node_increment[DIR_Z];
        film_element =
            modeling_data->joint_film.head +
            (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + 2) * element_increment[DIR_Y] +
            element_increment[DIR_X] +
            element_increment[DIR_Z];
        face1[0] = quad_node;
        face1[1] = quad_node + node_increment[DIR_X];
        face1[2] = quad_node + node_increment[DIR_X] + node_increment[DIR_Y];
        face1[3] = quad_node + node_increment[DIR_Y];
        face2[0] = hexa_node;
        face2[1] = hexa_node + node_increment[DIR_X];
        face2[2] = hexa_node + node_increment[DIR_X] + node_increment[DIR_Y];
        face2[3] = hexa_node + node_increment[DIR_Y];

        print_FILM_node(f, film_element, face1, face2, 1);
        print_COPYELM(
            f,
            film_element,
            0,
            0,
            element_increment[DIR_X],
            node_increment[DIR_X],
            modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X] - 1
        );
        if(element_set > 1) {
            print_COPYELM(
                f,
                film_element,
                film_element + (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X] - 1) * element_increment[DIR_X],
                element_increment[DIR_X],
                element_increment[DIR_Y],
                node_increment[DIR_Y],
                element_set - 1
            );
        }
        // 梁ウェブ - 上
        quad_node =
            modeling_data->joint_quad.head.node +
            (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + 1) * node_increment[DIR_Y] + 
            (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z]) * node_increment[DIR_Z];
        hexa_node =
            modeling_data->column_hexa.head.node +
            (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + 1) * node_increment[DIR_Y] +
            (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_START_Z] + 1) * node_increment[DIR_Z];
        film_element =
            modeling_data->joint_film.head +
            (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + 2) * element_increment[DIR_Y] +
            element_increment[DIR_X] +
            (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] + 2) * element_increment[DIR_Z];
        face1[0] = quad_node;
        face1[1] = quad_node + node_increment[DIR_Y];
        face1[2] = quad_node + node_increment[DIR_X] + node_increment[DIR_Y];
        face1[3] = quad_node + node_increment[DIR_X];
        face2[0] = hexa_node;
        face2[1] = hexa_node + node_increment[DIR_Y];
        face2[2] = hexa_node + node_increment[DIR_X] + node_increment[DIR_Y];
        face2[3] = hexa_node + node_increment[DIR_X];

        print_FILM_node(f, film_element, face1, face2, 1);
        print_COPYELM(
            f,
            film_element,
            0,
            0,
            element_increment[DIR_X],
            node_increment[DIR_X],
            modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X] - 1
        );
        if(element_set > 1) {
            print_COPYELM(
                f,
                film_element,
                film_element + (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X] - 1) * element_increment[DIR_X],
                element_increment[DIR_X],
                element_increment[DIR_Y],
                node_increment[DIR_Y],
                element_set - 1
            );
        }
    }   

    // 右
    quad_node =
        modeling_data->joint_quad.head.node + 
        (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X] - 1) * node_increment[DIR_X];
    hexa_node =
        modeling_data->column_hexa.head.node +
        (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * node_increment[DIR_X] +
        (modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[COLUMN_START_Z] + 1) * node_increment[DIR_Z];
    film_element =
        modeling_data->joint_film.head +
        (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 2) * element_increment[DIR_X] +
        element_increment[DIR_Y] +
        element_increment[DIR_Z];
    face1[0] = quad_node;
    face1[1] = quad_node + node_increment[DIR_X];
    face1[2] = quad_node + node_increment[DIR_X] + node_increment[DIR_Y];
    face1[3] = quad_node + node_increment[DIR_Y];
    face2[0] = hexa_node;
    face2[1] = hexa_node - node_increment[DIR_Z];
    face2[2] = hexa_node + node_increment[DIR_Y] - node_increment[DIR_Z];
    face2[3] = hexa_node + node_increment[DIR_Y];
    print_FILM_node(f, film_element, face1, face2, 1);
    pre_element = film_element;
    // 角
    quad_node +=
        (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y];
    hexa_node +=
        (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y];
    film_element +=
        (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * element_increment[DIR_Y];
    face1[0] = quad_node;
    face1[1] = quad_node + node_increment[DIR_X];
    face1[2] = quad_node + node_increment[DIR_X] + node_increment[DIR_Y];
    face1[3] = quad_node + node_increment[DIR_Y];
    face2[0] = hexa_node;
    face2[1] = hexa_node - node_increment[DIR_Z];
    face2[2] = hexa_node + node_increment[DIR_X] + node_increment[DIR_Y];
    face2[3] = hexa_node + node_increment[DIR_Y];
    print_FILM_node(f, film_element, face1, face2, 1);
    // 上
    quad_node =
        modeling_data->joint_quad.head.node + 
        (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X] - 1) * node_increment[DIR_X] +
        (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z]) * node_increment[DIR_Z];
    hexa_node =
        modeling_data->column_hexa.head.node +
        (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * node_increment[DIR_X] +
        (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_START_Z] + 1) * node_increment[DIR_Z];
    film_element =
        modeling_data->joint_film.head +
        (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 2) * element_increment[DIR_X] +
        element_increment[DIR_Y] +
        (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] + 2) * element_increment[DIR_Z];
    face1[0] = quad_node;
    face1[1] = quad_node + node_increment[DIR_Y];
    face1[2] = quad_node + node_increment[DIR_X] + node_increment[DIR_Y];
    face1[3] = quad_node + node_increment[DIR_X];
    face2[0] = hexa_node;
    face2[1] = hexa_node + node_increment[DIR_Y];
    face2[2] = hexa_node + node_increment[DIR_Y] + node_increment[DIR_Z];
    face2[3] = hexa_node + node_increment[DIR_Z];
    print_FILM_node(f, film_element, face1, face2, 1);
    print_COPYELM(
        f,
        pre_element,
        film_element,
        film_element - pre_element,
        element_increment[DIR_Y],
        node_increment[DIR_Y],
        modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] - 1
    );
    // 角
    quad_node +=
        (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y];
    hexa_node +=
        (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y];
    film_element +=
        (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * element_increment[DIR_Y];
    face1[0] = quad_node;
    face1[1] = quad_node + node_increment[DIR_Y];
    face1[2] = quad_node + node_increment[DIR_X] + node_increment[DIR_Y];
    face1[3] = quad_node + node_increment[DIR_X];
    face2[0] = hexa_node;
    face2[1] = hexa_node + node_increment[DIR_Y];
    face2[2] = hexa_node + node_increment[DIR_X] + node_increment[DIR_Y];
    face2[3] = hexa_node + node_increment[DIR_Z];
    print_FILM_node(f, film_element, face1, face2, 1);
    // 梁ウェブ
    quad_node =
        modeling_data->joint_quad.head.node + 
        (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * node_increment[DIR_X] +
        (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y];
    hexa_node =
        modeling_data->column_hexa.head.node +
        (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * node_increment[DIR_X] +
        (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * node_increment[DIR_Y] +
        (modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[COLUMN_START_Z]) * node_increment[DIR_Z];
    film_element =
        modeling_data->joint_film.head +
        (modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 3) * element_increment[DIR_X] +
        (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + 1) * element_increment[DIR_Y] +
        element_increment[DIR_Z];
    face1[0] = quad_node;
    face1[1] = quad_node + node_increment[DIR_X];
    face1[2] = quad_node + node_increment[DIR_X] + node_increment[DIR_Y];
    face1[3] = quad_node + node_increment[DIR_Y];
    face2[0] = hexa_node;
    face2[1] = hexa_node + node_increment[DIR_X];
    face2[2] = hexa_node + 2 * node_increment[DIR_X] + node_increment[DIR_Y] + node_increment[DIR_Z];
    face2[3] = hexa_node + node_increment[DIR_X] + node_increment[DIR_Y] + node_increment[DIR_Z];
    print_FILM_node(f, film_element, face1, face2, 1);
    pre_element = film_element;
    quad_node +=
        (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z]) * node_increment[DIR_Z];
    hexa_node +=
        (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] + 2) * node_increment[DIR_Z];
    film_element +=
        (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] + 1) * element_increment[DIR_Z];
    face1[0] = quad_node;
    face1[1] = quad_node + node_increment[DIR_Y];
    face1[2] = quad_node + node_increment[DIR_X] + node_increment[DIR_Y];
    face1[3] = quad_node + node_increment[DIR_X];
    face2[0] = hexa_node;
    face2[1] = hexa_node + node_increment[DIR_X] + node_increment[DIR_Y] - node_increment[DIR_Z];
    face2[2] = hexa_node + 2 * node_increment[DIR_X] + node_increment[DIR_Y] - node_increment[DIR_Z];
    face2[3] = hexa_node + node_increment[DIR_X];
    print_FILM_node(f, film_element, face1, face2, 1);
    print_COPYELM(
        f,
        pre_element,
        film_element,
        film_element - pre_element,
        element_increment[DIR_X],
        node_increment[DIR_X],
        modeling_data->boundary_index[COLUMN_BEAM_X] - modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - 1
    );

    // 直交梁内部 - 下
    element_set = modeling_data->boundary_index[ORTHOGONAL_BEAM_COLUMN_X] - modeling_data->boundary_index[COLUMN_CENTER_X] - 1;
    if(element_set > 0) {
        quad_node =
            modeling_data->joint_quad.head.node +
            (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * node_increment[DIR_X];
        hexa_node =
            modeling_data->column_hexa.head.node +
            (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 1) * node_increment[DIR_X] +
            (modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[COLUMN_START_Z] + 1) * node_increment[DIR_Z];
        film_element =
            modeling_data->joint_film.head +
            (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 3) * element_increment[DIR_X] +
            element_increment[DIR_Y] +
            element_increment[DIR_Z];
        face1[0] = quad_node;
        face1[1] = quad_node + node_increment[DIR_X];
        face1[2] = quad_node + node_increment[DIR_X] + node_increment[DIR_Y];
        face1[3] = quad_node + node_increment[DIR_Y];
        face2[0] = hexa_node;
        face2[1] = hexa_node + node_increment[DIR_X];
        face2[2] = hexa_node + node_increment[DIR_X] + node_increment[DIR_Y];
        face2[3] = hexa_node + node_increment[DIR_Y];
        print_FILM_node(f, film_element, face1, face2, 1);
        print_COPYELM(
            f,
            film_element,
            0,
            0,
            element_increment[DIR_Y],
            node_increment[DIR_Y],
            modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]
        );
        if(element_set > 1) {
            print_COPYELM(
                f,
                film_element,
                film_element + (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * element_increment[DIR_Y],
                element_increment[DIR_Y],
                element_increment[DIR_X],
                node_increment[DIR_X],
                element_set - 1
            );
        }
        // 内部 - 上
        quad_node =
            modeling_data->joint_quad.head.node +
            (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * node_increment[DIR_X] +
            (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z]) * node_increment[DIR_Z];
        hexa_node =
            modeling_data->column_hexa.head.node +
            (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 1) * node_increment[DIR_X] +
            (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_START_Z] + 1) * node_increment[DIR_Z];
        film_element =
            modeling_data->joint_film.head +
            (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 3) * element_increment[DIR_X] +
            element_increment[DIR_Y] +
            (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] + 2) * element_increment[DIR_Z];
        face1[0] = quad_node;
        face1[1] = quad_node + node_increment[DIR_Y];
        face1[2] = quad_node + node_increment[DIR_X] + node_increment[DIR_Y];
        face1[3] = quad_node + node_increment[DIR_X];
        face2[0] = hexa_node;
        face2[1] = hexa_node + node_increment[DIR_Y];
        face2[2] = hexa_node + node_increment[DIR_X] + node_increment[DIR_Y];
        face2[3] = hexa_node + node_increment[DIR_X];

        print_FILM_node(f, film_element, face1, face2, 1);
        print_COPYELM(
            f,
            film_element,
            0,
            0,
            element_increment[DIR_Y],
            node_increment[DIR_Y],
            modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]
        );
        if(element_set > 1) {
            print_COPYELM(
                f,
                film_element,
                film_element + (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * element_increment[DIR_Y],
                element_increment[DIR_Y],
                element_increment[DIR_X],
                node_increment[DIR_X],
                element_set - 1
            );
        }
    }
    // 梁内部 - 下
    element_set = modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_BEAM_Y] - 1;
    if(element_set > 0) {
        // 梁ウェブ - 下
        quad_node =
            modeling_data->joint_quad.head.node +
            (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * node_increment[DIR_X] +
            (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + 1) * node_increment[DIR_Y];
        hexa_node =
            modeling_data->column_hexa.head.node +
            (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 1) * node_increment[DIR_X] +
            (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + 1) * node_increment[DIR_Y] +
            (modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[COLUMN_START_Z] + 1) * node_increment[DIR_Z];
        film_element =
            modeling_data->joint_film.head +
            (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 3) * element_increment[DIR_X] +
            (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + 2) * element_increment[DIR_Y] +
            element_increment[DIR_Z];
        face1[0] = quad_node;
        face1[1] = quad_node + node_increment[DIR_X];
        face1[2] = quad_node + node_increment[DIR_X] + node_increment[DIR_Y];
        face1[3] = quad_node + node_increment[DIR_Y];
        face2[0] = hexa_node;
        face2[1] = hexa_node + node_increment[DIR_X];
        face2[2] = hexa_node + node_increment[DIR_X] + node_increment[DIR_Y];
        face2[3] = hexa_node + node_increment[DIR_Y];

        print_FILM_node(f, film_element, face1, face2, 1);
        print_COPYELM(
            f,
            film_element,
            0,
            0,
            element_increment[DIR_X],
            node_increment[DIR_X],
            modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X] - 1
        );
        if(element_set > 1) {
            print_COPYELM(
                f,
                film_element,
                film_element + (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X] - 1) * element_increment[DIR_X],
                element_increment[DIR_X],
                element_increment[DIR_Y],
                node_increment[DIR_Y],
                element_set - 1
            );
        }
        // 梁ウェブ - 上
        quad_node =
            modeling_data->joint_quad.head.node +
            (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * node_increment[DIR_X] +
            (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + 1) * node_increment[DIR_Y] + 
            (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z]) * node_increment[DIR_Z];
        hexa_node =
            modeling_data->column_hexa.head.node +
            (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 1) * node_increment[DIR_X] +
            (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + 1) * node_increment[DIR_Y] +
            (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_START_Z] + 1) * node_increment[DIR_Z];
        film_element =
            modeling_data->joint_film.head +
            (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + 2) * element_increment[DIR_Y] +
            (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 3) * element_increment[DIR_X] +
            (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] + 2) * element_increment[DIR_Z];
        face1[0] = quad_node;
        face1[1] = quad_node + node_increment[DIR_Y];
        face1[2] = quad_node + node_increment[DIR_X] + node_increment[DIR_Y];
        face1[3] = quad_node + node_increment[DIR_X];
        face2[0] = hexa_node;
        face2[1] = hexa_node + node_increment[DIR_Y];
        face2[2] = hexa_node + node_increment[DIR_X] + node_increment[DIR_Y];
        face2[3] = hexa_node + node_increment[DIR_X];

        print_FILM_node(f, film_element, face1, face2, 1);
        print_COPYELM(
            f,
            film_element,
            0,
            0,
            element_increment[DIR_X],
            node_increment[DIR_X],
            modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X] - 1
        );
        if(element_set > 1) {
            print_COPYELM(
                f,
                film_element,
                film_element + (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X] - 1) * element_increment[DIR_X],
                element_increment[DIR_X],
                element_increment[DIR_Y],
                node_increment[DIR_Y],
                element_set - 1
            );
        }
    }   
    fprintf(f, "\n");
}


void add_beam_hexa(FILE *f, ModelingData *modeling_data) {
    fprintf(f, "---- BEAM HEXA ----\n");
    // ポインタ配列に各方向を格納
    NodeCoordinate* coordinates[3] = {modeling_data->x, modeling_data->y, modeling_data->z};

    int node_increment[3] = {
        modeling_data->beam.increment[0].node,
        modeling_data->beam.increment[1].node,
        modeling_data->beam.increment[2].node
    };
    int element_increment[3] = {
        modeling_data->beam.increment[0].element,
        modeling_data->beam.increment[1].element,
        modeling_data->beam.increment[2].element
    };

    // 左 -----------------------------------------------------
    int start[3] = {
        modeling_data->boundary_index[BEAM_START_X],
        modeling_data->boundary_index[COLUMN_BEAM_Y],
        modeling_data->boundary_index[COLUMN_BEAM_Z]
    };
    int end[3] = {
        modeling_data->boundary_index[JIG_BEAM_X],
        modeling_data->boundary_index[CENTER_Y],
        modeling_data->boundary_index[BEAM_COLUMN_Z]
    };
    
    generate_hexa(f, coordinates, modeling_data->beam.head.node, modeling_data->beam.head.element, start, end, node_increment, element_increment, 5);
    // 右 -----------------------------------------------------
    int start_node =
        modeling_data->beam.head.node +
        (modeling_data->boundary_index[BEAM_JIG_X] - modeling_data->boundary_index[BEAM_START_X]) * node_increment[DIR_X];
    int start_element =
        modeling_data->beam.head.element +
        (modeling_data->boundary_index[BEAM_JIG_X] - modeling_data->boundary_index[BEAM_START_X]) * element_increment[DIR_X];

    start[DIR_X] = modeling_data->boundary_index[BEAM_JIG_X];
    end[DIR_X] = modeling_data->boundary_index[BEAM_END_X];

    generate_hexa(f, coordinates, start_node, start_element, start, end, node_increment, element_increment, 5);

}

void add_beam_quad(FILE *f, ModelingData *modeling_data) {

    fprintf(f, "---- BEAM QUAD ----\n");
    // ポインタ配列に各方向を格納
    NodeCoordinate* coordinates[3] = {modeling_data->x, modeling_data->y, modeling_data->z};

    int node_increment[3] = {
        modeling_data->beam.increment[0].node,
        modeling_data->beam.increment[1].node,
        modeling_data->beam.increment[2].node
    };
    int element_increment[3] = {
        modeling_data->beam.increment[0].element,
        modeling_data->beam.increment[1].element,
        modeling_data->beam.increment[2].element
    };

    // 節点定義
    if(modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] > 2) {
        // 0 : 左, 1 : 右
        for(int i = 0; i < 2; i++) {
            BoundaryType boundary_type = JIG_BEAM_X;
            if(i == 1) {
                boundary_type = COLUMN_BEAM_X;
            }
            // 下フランジ
            int start_node =
                modeling_data->beam.head.node +
                (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[BEAM_START_X] + 1) * node_increment[DIR_X];
            int start[3] = {
                modeling_data->boundary_index[boundary_type] + 1,
                modeling_data->boundary_index[COLUMN_BEAM_Y],
                modeling_data->boundary_index[COLUMN_BEAM_Z]
            };
            int end[3] = {
                modeling_data->boundary_index[boundary_type + 1] - 1,
                modeling_data->boundary_index[CENTER_Y],
                modeling_data->boundary_index[COLUMN_BEAM_Z]
            };
            plot_node(f, coordinates, start_node, start, end, node_increment);
            // 上フランジ
            start_node =
                modeling_data->beam.head.node +
                (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[BEAM_START_X] + 1) * node_increment[DIR_X] +
                (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z]) * node_increment[DIR_Z];
            start[DIR_Z] = modeling_data->boundary_index[BEAM_COLUMN_Z];
            end[DIR_Z] = start[DIR_Z];
            plot_node(f, coordinates, start_node, start, end, node_increment);
            
            // ウェブ
            start_node =
                modeling_data->beam.head.node +
                (modeling_data->boundary_index[boundary_type] - modeling_data->boundary_index[BEAM_START_X] + 1) * node_increment[DIR_X] +
                (modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_BEAM_Y]) * node_increment[DIR_Y] +
                node_increment[DIR_Z];

            start[DIR_Y] = modeling_data->boundary_index[CENTER_Y];
            start[DIR_Z] = modeling_data->boundary_index[COLUMN_BEAM_Z] + 1;
            end[DIR_Y] = modeling_data->boundary_index[CENTER_Y];
            end[DIR_Z] = modeling_data->boundary_index[BEAM_COLUMN_Z] - 1;
            plot_node(f, coordinates, start_node, start, end, node_increment);
        }
    }

    // 接合部
    // フランジ - フルモデルは考慮していない
    int quad_diff = (modeling_data->boundary_index[COLUMN_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X] + 1) + node_increment[DIR_X];
    int joint_diff = (modeling_data->boundary_index[COLUMN_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * modeling_data->column_hexa.increment[DIR_X].node;
    int element_diff = (modeling_data->boundary_index[COLUMN_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) + element_increment[DIR_X];
    for(int i = 0; i < modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_BEAM_Y]; i++) {
        // 下フランジ
        int element =
            modeling_data->beam.head.element +
            (modeling_data->boundary_index[BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_START_X] - 1) * element_increment[DIR_X] +
            i * element_increment[DIR_Y];
        int quad_start =
            modeling_data->beam.head.node +
            (modeling_data->boundary_index[BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_START_X] - 1) * node_increment[DIR_X] +
            (i + 1) * node_increment[DIR_Y];
        int joint_start =
            modeling_data->joint_quad.head.node +
            (modeling_data->boundary_index[COLUMN_BEAM_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] + i) * modeling_data->column_hexa.increment[DIR_Y].node;
        int node[4] = {
            quad_start,
            quad_start - node_increment[DIR_Y],
            joint_start,
            joint_start + modeling_data->column_hexa.increment[DIR_Y].node
        };
        print_QUAD_node(f, element, node, 4);

        node[0] = quad_start + quad_diff;
        node[1] = joint_start + modeling_data->column_hexa.increment[DIR_Y].node + joint_diff;
        node[2] = joint_start + joint_diff;
        node[3] = quad_start - node_increment[DIR_Y] + quad_diff;
        print_QUAD_node(f, element + element_diff, node, 4);

        // 上フランジ
        element += (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] + 1) * element_increment[DIR_Z];
        quad_start += (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z]) * node_increment[DIR_Z];
        joint_start += (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z]) * modeling_data->column_hexa.increment[DIR_Z].node;
        node[0] = quad_start;
        node[1] = quad_start - node_increment[DIR_Y];
        node[2] = joint_start;
        node[3] = joint_start + modeling_data->column_hexa.increment[DIR_Y].node;
        print_QUAD_node(f, element, node, 3);

        node[0] = quad_start + quad_diff;
        node[1] = joint_start + modeling_data->column_hexa.increment[DIR_Y].node + joint_diff;
        node[2] = joint_start + joint_diff;
        node[3] = quad_start - node_increment[DIR_Y] + quad_diff;
        print_QUAD_node(f, element + element_diff, node, 3);

    }

    // ウェブ
    for(int i = 0; i < modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z]; i++) {
        int element =
            modeling_data->beam.head.element +
            (modeling_data->boundary_index[BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_START_X] - 1) * element_increment[DIR_X] +
            (modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_BEAM_Y]) * element_increment[DIR_Y] +
            (i + 1) * element_increment[DIR_Z];
        int quad_start =
            modeling_data->beam.head.node +
            (modeling_data->boundary_index[BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_START_X] - 1) * node_increment[DIR_X] +
            (modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_BEAM_Y]) * node_increment[DIR_Y] +
            i * node_increment[DIR_Z];
        int joint_start =
            modeling_data->joint_quad.head.node +
            (modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * modeling_data->column_hexa.increment[DIR_Y].node +
            i * modeling_data->column_hexa.increment[DIR_Z].node;
        int node[4] = {
            quad_start,
            quad_start + node_increment[DIR_Z],
            joint_start + modeling_data->column_hexa.increment[DIR_Z].node,
            joint_start
            
        };
        print_QUAD_node(f, element, node, 1);

        node[0] = quad_start + quad_diff;
        node[1] = joint_start + joint_diff;
        node[2] = joint_start + joint_diff + modeling_data->column_hexa.increment[DIR_Z].node;
        node[3] = quad_start + node_increment[DIR_Z] + quad_diff;
        print_QUAD_node(f, element + element_diff, node, 1);

    }

    // 梁
    if(modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] > 2) {
        for(int i = 0; i < 2; i++) {
            int x_index = modeling_data->boundary_index[JIG_BEAM_X];
            int x_copy_num = modeling_data->boundary_index[BEAM_COLUMN_X] - modeling_data->boundary_index[JIG_BEAM_X] - 2;
            if(i == 1) {
                x_index = modeling_data->boundary_index[COLUMN_BEAM_X] + 1;
                x_copy_num = modeling_data->boundary_index[BEAM_JIG_X] - modeling_data->boundary_index[COLUMN_BEAM_X] - 2;
            }

            int element =
                modeling_data->beam.head.element + x_index * element_increment[DIR_X];
                
            int node[4] = {
                modeling_data->beam.head.node + x_index * node_increment[DIR_X],
                modeling_data->beam.head.node + (x_index + 1 ) * node_increment[DIR_X],
                modeling_data->beam.head.node + (x_index + 1 ) * node_increment[DIR_X] + node_increment[DIR_Y],
                modeling_data->beam.head.node + x_index * node_increment[DIR_X] + node_increment[DIR_Y],
            };
            print_QUAD_node(f, element, node, 4);
            // y方向
            print_COPYELM(f, element, 0, 0, element_increment[DIR_Y], node_increment[DIR_Y], modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_BEAM_Y] - 1);
            // x方向
            print_COPYELM(
                f,
                element,
                element + (modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_BEAM_Y] - 1) * element_increment[DIR_Y],
                element_increment[DIR_Y],
                element_increment[DIR_X],
                node_increment[DIR_X],
                x_copy_num
            );
            
            element += (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] + 1) * element_increment[DIR_Z];
            for(int i = 0; i < 4; i++) {
                node[i] += (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z]) * node_increment[DIR_Z];
            }
            print_QUAD_node(f, element, node, 3);
            print_COPYELM(f, element, 0, 0, element_increment[DIR_Y], node_increment[DIR_Y], modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_BEAM_Y] - 1);
            print_COPYELM(
                f,
                element,
                element + (modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_BEAM_Y] - 1) * element_increment[DIR_Y],
                element_increment[DIR_Y],
                element_increment[DIR_X],
                node_increment[DIR_X],
                x_copy_num
            );

            element =
                modeling_data->beam.head.element +
                x_index * element_increment[DIR_X] +
                (modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_BEAM_Y]) * element_increment[DIR_Y] +
                element_increment[DIR_Z];

            node[0] =
                modeling_data->beam.head.node + x_index * node_increment[DIR_X] +
                (modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_BEAM_Y]) * node_increment[DIR_Y];
            node[1] = node[0] + node_increment[DIR_Z];
            node[2] = node[0] + node_increment[DIR_X] + node_increment[DIR_Z];
            node[3] = node[0] + node_increment[DIR_X];
            print_QUAD_node(f, element, node, 1);
            print_COPYELM(f, element, 0, 0, element_increment[DIR_Z], node_increment[DIR_Z], modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1);
            print_COPYELM(
                f,
                element,
                element + (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z] - 1) * element_increment[DIR_Z],
                element_increment[DIR_Z],
                element_increment[DIR_X],
                node_increment[DIR_X],
                x_copy_num
            );
        }
    }
    fprintf(f, "\n");
}


/**
 * 柱、梁を選択して端部をピン指示にする。
 */
void set_pin(FILE *f, ModelingData *modeling_data, char parts) {
    fprintf(f, "---- SET PIN ----\n");
    if(parts == 'b' || parts == 'B') {
        int start_node = modeling_data->beam.head.node;
        print_REST(
            f,
            start_node,
            start_node + (modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_BEAM_Y]) * modeling_data->beam.increment[DIR_Y].node,
            modeling_data->beam.increment[DIR_Y].node,
            001,
            modeling_data->beam.increment[DIR_Z].node,
            (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z])
        );
        start_node += (modeling_data->boundary_index[BEAM_END_X] - modeling_data->boundary_index[BEAM_START_X]) * modeling_data->beam.increment[DIR_X].node;
        print_REST(
            f,
            start_node,
            start_node + (modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_BEAM_Y]) * modeling_data->beam.increment[DIR_Y].node,
            modeling_data->beam.increment[DIR_Y].node,
            001,
            modeling_data->beam.increment[DIR_Z].node,
            (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z])
        );
    } else {
        printf("non\n");
        return ;
    }
    fprintf(f, "\n");
}

void set_roller(FILE *f, ModelingData *modeling_data, char parts) {
    fprintf(f, "---- SET ROLLER ----\n");
    if(parts == 'c' || parts == 'C') {
        // 柱、下端
        int center = modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X];
        int master_bottom =
            modeling_data->column_hexa.head.node +
            (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * modeling_data->column_hexa.increment[DIR_X].node +
            (modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * modeling_data->column_hexa.increment[DIR_Y].node;
        
        for(int i = 0; i < modeling_data->boundary_index[COLUMN_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X]; i++) {
            int node = modeling_data->column_hexa.head.node + i * modeling_data->column_hexa.increment[DIR_X].node;
            if(i != center) {
                print_SUB1(
                    f,
                    node,
                    node + (modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * modeling_data->column_hexa.increment[DIR_Y].node,
                    modeling_data->column_hexa.increment[DIR_Y].node,
                    1,
                    master_bottom,
                    1
                );
            } else {
                print_SUB1(
                    f,
                    node,
                    node + (modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] - 1) * modeling_data->column_hexa.increment[DIR_Y].node,
                    modeling_data->column_hexa.increment[DIR_Y].node,
                    1,
                    master_bottom,
                    1
                );
            }
        }
        // 柱、上端
        int master_top =
            master_bottom + (modeling_data->boundary_index[COLUMN_END_Z] - modeling_data->boundary_index[COLUMN_START_Z] + 2) * modeling_data->column_hexa.increment[DIR_Z].node;

        for(int i = 0; i < modeling_data->boundary_index[COLUMN_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X]; i++) {
            int node =
                modeling_data->column_hexa.head.node + i * modeling_data->column_hexa.increment[DIR_X].node +
                (modeling_data->boundary_index[COLUMN_END_Z] - modeling_data->boundary_index[COLUMN_START_Z] + 2) * modeling_data->column_hexa.increment[DIR_Z].node;
            if(i != center) {
                print_SUB1(
                    f,
                    node,
                    node + (modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * modeling_data->column_hexa.increment[DIR_Y].node,
                    modeling_data->column_hexa.increment[DIR_Y].node,
                    1,
                    master_top,
                    1
                );
            } else {
                print_SUB1(
                    f,
                    node,
                    node + (modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y] - 1) * modeling_data->column_hexa.increment[DIR_Y].node,
                    modeling_data->column_hexa.increment[DIR_Y].node,
                    1,
                    master_top,
                    1
                );
            }
        }

    } else {
        printf("non\n");
        return ;
    }
    fprintf(f, "\n");
}

void fix_cut_surface(FILE *f, ModelingData *modeling_data) {
    fprintf(f, "---- FIX CUT SURFACE ----\n");
    // 柱
    int start_node =
        modeling_data->column_hexa.head.node +
        (modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * modeling_data->column_hexa.increment[DIR_Y].node;
    print_REST(
        f,
        start_node,
        start_node + (modeling_data->boundary_index[COLUMN_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * modeling_data->column_hexa.increment[DIR_X].node,
        modeling_data->column_hexa.increment[DIR_X].node,
        10,
        modeling_data->column_hexa.increment[DIR_Z].node,
        modeling_data->boundary_index[COLUMN_BEAM_Z] - modeling_data->boundary_index[COLUMN_START_Z]
    );
    start_node += (modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_START_Z] + 2) * modeling_data->column_hexa.increment[DIR_Z].node;
    print_REST(
        f,
        start_node,
        start_node + (modeling_data->boundary_index[COLUMN_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * modeling_data->column_hexa.increment[DIR_X].node,
        modeling_data->column_hexa.increment[DIR_X].node,
        10,
        modeling_data->column_hexa.increment[DIR_Z].node,
        modeling_data->boundary_index[COLUMN_END_Z] - modeling_data->boundary_index[BEAM_COLUMN_Z]
    );
    // 梁
    start_node =
        modeling_data->beam.head.node +
        (modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_BEAM_Y]) * modeling_data->beam.increment[DIR_Y].node;
    print_REST(
        f,
        start_node,
        start_node + (modeling_data->boundary_index[BEAM_COLUMN_X] - modeling_data->boundary_index[BEAM_START_X] - 1) * modeling_data->beam.increment[DIR_X].node,
        modeling_data->beam.increment[DIR_X].node,
        10,
        modeling_data->beam.increment[DIR_Z].node,
        modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z]
    );
    start_node =
        modeling_data->beam.head.node +
        (modeling_data->boundary_index[COLUMN_BEAM_X] - modeling_data->boundary_index[BEAM_START_X] + 1) * modeling_data->beam.increment[DIR_X].node +
        (modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_BEAM_Y]) * modeling_data->beam.increment[DIR_Y].node;
    print_REST(
        f,
        start_node,
        start_node + (modeling_data->boundary_index[BEAM_END_X] - modeling_data->boundary_index[COLUMN_BEAM_X] - 1) * modeling_data->beam.increment[DIR_X].node,
        modeling_data->beam.increment[DIR_X].node,
        10,
        modeling_data->beam.increment[DIR_Z].node,
        modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z]
    );
    start_node =
        modeling_data->joint_quad.head.node +
        (modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * modeling_data->column_hexa.increment[DIR_Y].node;
    print_REST(
        f,
        start_node,
        start_node + (modeling_data->boundary_index[COLUMN_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * modeling_data->column_hexa.increment[DIR_X].node,
        modeling_data->beam.increment[DIR_X].node,
        10,
        modeling_data->column_hexa.increment[DIR_Z].node,
        modeling_data->boundary_index[BEAM_COLUMN_Z] - modeling_data->boundary_index[COLUMN_BEAM_Z]
    );
    fprintf(f, "\n");
}

void get_load_node(ModelingData *modeling_data, int load_nodes[]) {
    load_nodes[0] =
        modeling_data->column_hexa.head.node +
        (modeling_data->boundary_index[COLUMN_CENTER_X] - modeling_data->boundary_index[BEAM_COLUMN_X]) * modeling_data->column_hexa.increment[DIR_X].node +
        (modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y]) * modeling_data->column_hexa.increment[DIR_Y].node;
    load_nodes[1] =
        load_nodes[0] + (modeling_data->boundary_index[COLUMN_END_Z] - modeling_data->boundary_index[COLUMN_START_Z] + 2) * modeling_data->column_hexa.increment[DIR_Z].node;
}

void print_type_mat(FILE *f) {
    // 要素タイプ
    print_TYPH(f, 1, 1, 'c');
    print_TYPH(f, 2, 1, 'c');
    print_TYPH(f, 3, 1, 'c');
    print_TYPH(f, 4, 1, 'c');
    print_TYPH(f, 5, 1, 's');
    print_TYPB(f, 1, 2);
    print_TYPL(f, 1, 1, 1);

    print_TYPQ(f, 1, 1);
    print_TYPQ(f, 2, 1);
    print_TYPQ(f, 3, 1);
    print_TYPQ(f, 4, 1);
    print_TYPQ(f, 5, 1);
    print_TYPQ(f, 6, 1);
    print_TYPQ(f, 7, 1);
    print_TYPQ(f, 8, 1);
    print_TYPQ(f, 9, 1);
    print_TYPQ(f, 10, 1);

    print_TYPF(f, 1, 1);
    print_TYPF(f, 2, 1);
    print_AXIS(f, 1);
    fprintf(f, "\n");
    // 材料モデル
    print_MATC(f, 1);
    print_MATS(f, 1);
    print_MATS(f, 2);
    print_MATJ(f, 1);
    fprintf(f, "\n");
}

/**
 * 軸力導入するstepデータを書き込む
 */
void print_axial_force_step(FILE *f, ModelingData *modeling_data) {
    double unit = 10;
    print_STEP(f, 1);

    int element_index = modeling_data->column_hexa.head.element;
    int element_increment_x = modeling_data->column_hexa.increment[DIR_X].element;
    int element_increment_y = modeling_data->column_hexa.increment[DIR_Y].element;
    // 柱のx方向要素数
    int element_num_x = modeling_data->boundary_index[COLUMN_BEAM_X] - modeling_data->boundary_index[BEAM_COLUMN_X];
    // 柱のy方向要素数
    int element_num_y = modeling_data->boundary_index[CENTER_Y] - modeling_data->boundary_index[COLUMN_SURFACE_START_Y];
    int element_diff = (element_num_x - 1) * element_increment_x;
    // 柱下部
    for(int i = 0; i < element_num_y; i++) {
        int start_element = element_index + element_increment_y * i;
        print_UE(f, start_element, start_element + element_diff, element_increment_x, unit, 'z', 1);
    }

    element_index = modeling_data->column_hexa.head.element + 
        (modeling_data->boundary_index[COLUMN_END_Z] - modeling_data->boundary_index[COLUMN_START_Z] - 1) * modeling_data->column_hexa.increment[DIR_Z].element;
    // 柱上部
    for(int i = 0; i < element_num_y; i++) {
        int start_element = element_index + element_increment_y * i;
        print_UE(f, start_element, start_element + element_diff, element_increment_x, -1 * unit, 'z', 2);
    }
    print_OUT(f, 1, 0, 0);
    fprintf(f, "\n");
}

void print_load_step(FILE *f, int load_nodes[]) {
    print_STEP(f, 10);
    print_FN(f, load_nodes[0], 0, 0, -10, 'x');
    print_FN(f, load_nodes[1], 0, 0, 10, 'x');
    print_OUT(f, 2, 10, 1);
}


void debug_step_print(FILE *f) {
    fprintf(f, "REST :NODE  S(    1)-E(     )-I(     )  RC=(111111) INC(     )-SET(    )\n");
    fprintf(f, "STEP :UP TO NO.(    1)   MAXIMUM LOAD INCREMENT=         CREEP=(0)(0:NO)\n");
    fprintf(f, "  FN :NODE  S(    2)-E(     )-I(     )    FORCE=1        DIR(1)\n");
    fprintf(f, " OUT :STEP  S(    1)-E(     )-I(     ) LEVEL=(1) (1:RESULT 2:POST 3:1+2)\n");
}


#define OUT_FILE_NAME  "out.ffi"
/**
 * @param inputFileName rcsモデリングデータのファイル名
 * @param outputFileName
 */
ModelingRcsResult modeling_rcs(const char *inputFileName, const char *outputFileName) {
    /*名称
        source_data  : JSONファイルの入力データ
        modeling_data: モデリングに必要なデータ
   */

    /*メモ
        格子の原点は(0, 0, 0)
        主筋入力データの座標は柱左下を原点
        コマンドライン引数でフルの選択
        入力ファイル名の指定
        変数名　キャメルケース
        関数名　スネークケース
    */

   /** todo
    * 
    * 主筋データの順序
    * 
    */

    // JSONファイルの読み込み ---------------------------------------------------------------------
    JsonData* source_data = new_json_data();  // 初期化
	JsonParserResult result = json_parser(inputFileName, source_data);  // データ読み込み
	if (result == JSON_PARSER_SUCCESS) {
		print_json_data(source_data, 0);
	} else {
		printf("Failed to parse JSON.\n");
        free_json_data(source_data);
        return MODELING_RCS_ERROR;
	}
    
    // modeling_dataの作成 ---------------------------------------------------------------------
    // ModelingDataを初期化  原点(0)の分も要素数に加算
    ModelingData* modeling_data = create_modeling_data(
        source_data->mesh_x.mesh_num + 1,
        source_data->mesh_y.mesh_num + 1,
        source_data->mesh_z.mesh_num + 1,
        source_data->rebar.rebar_num
    );
    if (modeling_data == NULL) {
        fprintf(stderr, "Failed to create ModelingData\n");
        free_json_data(source_data);
        return MODELING_RCS_ERROR;
    }

    // データ格納 - source_dataはここで解放
    if(make_modeling_data(modeling_data, source_data) == EXIT_SUCCESS) {
        print_modeling_data(modeling_data);
        free_json_data(source_data);
    } else {
        printf("Failed to input for ModelingData\n");
        free_json_data(source_data);
        free_modeling_data(modeling_data);
        return MODELING_RCS_ERROR;
    }
    
    // ffiの書き込み -----------------------------------------------------------------
    //ファイルオープン
    FILE *fout = fopen(outputFileName,"w");
    if(fout == NULL)
    {
        printf("ERROR: out.ffi cant open.\n");
        free_modeling_data(modeling_data);
        return MODELING_RCS_ERROR;
    }

    // 強制変位を与える節点を取得
    int load_nodes[2] = {0};
    get_load_node(modeling_data, load_nodes);

    // 解析制御データ
    print_head_template(fout, 10, load_nodes[1], 'x', load_nodes[1], 'x');

    // 柱 - 六面体要素
    add_column_hexa(fout, modeling_data);

    // 柱主筋 - 線材要素,LINE要素
    add_reber_fiber_line(fout, modeling_data);

    //接合部 - 四辺形要素
    add_joint_quad(fout, modeling_data);
    //接合部 - FILM要素
    add_joint_film(fout, modeling_data);

    //梁 - 六面体要素
    add_beam_hexa(fout, modeling_data);
    //梁 - 四辺形要素
    add_beam_quad(fout, modeling_data);

    // 切断面拘束
    fix_cut_surface(fout, modeling_data);

    // 境界条件
    set_pin(fout, modeling_data, 'b');
    set_roller(fout, modeling_data, 'c');
    
    // 要素タイプ、材料モデル
    print_type_mat(fout);

    // 軸力導入
    print_axial_force_step(fout, modeling_data);

    // 強制変位
    print_load_step(fout, load_nodes);

    // END
    fprintf(fout, "\nEND\n");
    fclose(fout);

    free_modeling_data(modeling_data);  // メモリの解放
    return MODELING_RCS_SUCCESS;

}
