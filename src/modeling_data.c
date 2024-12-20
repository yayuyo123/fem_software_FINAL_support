#include "modeling_data.h"
#include <stdlib.h>

/**
 * メモリの確保と初期化は分離して行う方針
 * 
 */

// メモリ確保関数 ----------------------------------------------------------------------------
// NodeCoordinate用のメモリ確保関数
NodeCoordinate* allocate_node_coordinate(int num_nodes) {
    // NodeCoordinate 構造体のメモリを確保
    NodeCoordinate* node = (NodeCoordinate*)malloc(sizeof(NodeCoordinate));
    if (node == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for NodeCoordinate structure\n");
        return NULL;
    }

    // coordinate 配列のメモリを確保
    node->coordinate = (double*)malloc(num_nodes * sizeof(double));
    if (node->coordinate == NULL) {
        free(node);  // node のメモリを解放
        fprintf(stderr, "Error: Failed to allocate memory for coordinate array\n");
        return NULL;
    }

    // ノード数の設定
    node->node_num = num_nodes;

    // 正常に確保できた場合は、確保したポインタを返す
    return node;
}

// RebarFiber用のメモリ確保関数
RebarFiber* allocate_rebar_fiber(int rebar_num) {
    if (rebar_num <= 0) {
        fprintf(stderr, "Error: Invalid rebar_num (%d)\n", rebar_num);
        return NULL;
    }

    RebarFiber* rebar = (RebarFiber*)malloc(sizeof(RebarFiber));
    if (rebar == NULL) {
        fprintf(stderr, "Error: Memory allocation for RebarFiber failed\n");
        return NULL;
    }

    rebar->positions = (RebarPositionIndex*)malloc(rebar_num * sizeof(RebarPositionIndex));
    if (rebar->positions == NULL) {
        fprintf(stderr, "Error: Memory allocation for positions failed\n");
        free(rebar);
        return NULL;
    }

    rebar->rebar_num = rebar_num;
    return rebar;
}

// ModelingData用のメモリ確保関数
ModelingData* allocate_modeling_data() {
    ModelingData* data = (ModelingData*)malloc(sizeof(ModelingData));
    if (data == NULL) return NULL;

    return data;
}


// 初期化関数 ----------------------------------------------------------------------------

// NodeCoordinateを初期化
void initialize_node_coordinate(NodeCoordinate* node) {
    if (node == NULL) return;

    for (int i = 0; i < node->node_num; i++) {
        node->coordinate[i] = 0.0; // 初期値を設定
    }
}

// RebarFiberを初期化
void initialize_rebar_fiber(RebarFiber* rebar) {
    if (rebar == NULL || rebar->positions == NULL) {
        fprintf(stderr, "Error: Invalid RebarFiber or unallocated positions\n");
        return;
    }

    for (int i = 0; i < rebar->rebar_num; i++) {
        rebar->positions[i].x = 0;
        rebar->positions[i].y = 0;
    }
    // increment
    rebar->increment.node = 0;
    rebar->increment.element = 0;

    // occupied_indices
    rebar->occupied_indices.node = 0;
    rebar->occupied_indices.element = 0;

    // occupied_indices_single
    rebar->occupied_indices_single.node = 0;
    rebar->occupied_indices_single.element = 0;

    // head
    rebar->head.node = 0;
    rebar->head.element = 0;
}

// ColumnHexaを初期化
void initialize_column_hexa(ColumnHexa* column) {
    if(column == NULL) {
        return;
    }
    
    // increment
    for(int i = 0; i < 3; i++) {
        column->increment[i].node = 0;
        column->increment[i].element = 0;
    }
    // occupied_indices
    column->occupied_indices.node = 0;
    column->occupied_indices.element = 0;

    // head
    column->head.node = 0;
    column->head.element = 0;
}

//RebarLine
void initialize_rebar_line(RebarLine* rebar_line) {
    if(rebar_line == NULL) {
        return;
    }
    // increment
    rebar_line->increment.node = 0;
    rebar_line->increment.element = 0;

    // occupied_indices
    rebar_line->occupied_indices.node = 0;
    rebar_line->occupied_indices.element = 0;

    // occupied_indices_single
    rebar_line->occupied_indices_single.node = 0;
    rebar_line->occupied_indices_single.element = 0;

    // head
    rebar_line->head.node = 0;
    rebar_line->head.element = 0;
}

// joint_quad
void initialize_joint_quad(JointQuad* joint_quad) {

    for(int i = 0; i < 3; i++) {
        joint_quad->increment_element[i] = 0;
    }

    joint_quad->occupied_indices.node = 0;
    joint_quad->occupied_indices.element = 0;

    joint_quad->head.node = 0;
    joint_quad->head.element = 0;
}

// joint_film
void initialize_joint_film(JointFilm* joint_film) {

    joint_film->occupied_indices = 0;
    joint_film->head = 0;
}

// beam
void initialize_beam(BeamHexaQuad* beam) {

    for(int i = 0; i < 3; i++) {
        beam->increment[i].node = 0;
        beam->increment[i].element = 0;
    }
    beam->head.node = 0;
    beam->head.element = 0;
}

void initialize_modeling_data(ModelingData* data) {
    if (data == NULL) return;

    // x,y,z
    initialize_node_coordinate(data->x);
    initialize_node_coordinate(data->y);
    initialize_node_coordinate(data->z);

    // boundary_index
    for (int i = 0; i < BOUNDARY_X_MAX + BOUNDARY_Y_MAX + BOUNDARY_Z_MAX; i++) {
        data->boundary_index[i] = 0;
    }

    // column_hexa
    initialize_column_hexa(&data->column_hexa);

    // rebar_fiber
    initialize_rebar_fiber(data->rebar_fiber);

    // rebar_line
    initialize_rebar_line(&data->rebar_line);

    // joint_quad
    initialize_joint_quad(&data->joint_quad);

    // joint_film
    initialize_joint_film(&data->joint_film);

    // beam
    initialize_beam(&data->beam);
}

// メモリ解放関数 ----------------------------------------------------------------------------
/**
 * メモリは解放したらnullを入れる事
 */
// NodeCoordinateのメモリ解放
int free_node_coordinate(NodeCoordinate* node) {
    if (node == NULL) {
        fprintf(stderr, "Error: NULL pointer passed to free_node_coordinate\n");
        return EXIT_FAILURE;
    }

    // `node->coordinate`がNULLの場合の確認
    if (node->coordinate == NULL) {
        fprintf(stderr, "Warning: NodeCoordinate->coordinate is already NULL\n");
    } else {
        free(node->coordinate);
        node->coordinate = NULL; // 解放後にNULLを設定
        printf("NodeCoordinate->coordinate freed successfully\n");
    }

    free(node);

    return EXIT_SUCCESS;
}

// RebarFiberのメモリ解放
int free_rebar_fiber(RebarFiber* rebar) {
    if (rebar == NULL) {
        fprintf(stderr, "Error: NULL pointer passed to free_rebar_fiber\n");
        return EXIT_FAILURE;
    }

    // positionsの解放
    if (rebar->positions != NULL) {
        free(rebar->positions);
        rebar->positions = NULL; // 二重解放防止
        printf("RebarFiber->positions freed successfully\n");
    } else {
        fprintf(stderr, "Warning: RebarFiber->positions is already NULL\n");
    }

    // RebarFiber構造体自体の解放
    free(rebar);
    rebar = NULL; // 二重解放防止

    return EXIT_SUCCESS;
}

// ModelingDataのメモリ解放
int free_modeling_data(ModelingData* data) {
    if (data == NULL) {
        fprintf(stderr, "Error: NULL pointer passed to free_modeling_data\n");
        return EXIT_FAILURE;
    }

    int result = EXIT_SUCCESS; // 全体の終了ステータスを追跡

    // 各NodeCoordinateの解放
    if (free_node_coordinate(data->x) != EXIT_SUCCESS) {
        fprintf(stderr, "Error: Failed to free NodeCoordinate x\n");
        result = EXIT_FAILURE;
    } else {
        printf("NodeCoordinate x freed successfully\n");
    }

    if (free_node_coordinate(data->y) != EXIT_SUCCESS) {
        fprintf(stderr, "Error: Failed to free NodeCoordinate y\n");
        result = EXIT_FAILURE;
    } else {
        printf("NodeCoordinate y freed successfully\n");
    }

    if (free_node_coordinate(data->z) != EXIT_SUCCESS) {
        fprintf(stderr, "Error: Failed to free NodeCoordinate z\n");
        result = EXIT_FAILURE;
    } else {
        printf("NodeCoordinate z freed successfully\n");
    }

    // RebarFiberの解放
    if (free_rebar_fiber(data->rebar_fiber) != EXIT_SUCCESS) {
        fprintf(stderr, "Error: Failed to free RebarFiber\n");
        result = EXIT_FAILURE;
    } else {
        printf("RebarFiber freed successfully\n");
    }

    // ModelingData本体の解放
    free(data);

    return result;
}

// ModelingDataを作成する関数（メモリ確保 + 初期化）----------------------------------------------------------------------------
ModelingData* create_modeling_data(int x_node_num, int y_node_num, int z_node_num, int rebar_num) {
    // ModelingData本体のメモリ確保
    ModelingData* data = allocate_modeling_data();
    if (data == NULL) {
        fprintf(stderr, "Failed to allocate ModelingData\n");
        return NULL;
    }

    // 各NodeCoordinateのメモリ確保
    data->x = allocate_node_coordinate(x_node_num);
    data->y = allocate_node_coordinate(y_node_num);
    data->z = allocate_node_coordinate(z_node_num);

    if (data->x->coordinate == NULL || data->y->coordinate == NULL || data->z->coordinate == NULL) {
        fprintf(stderr, "Failed to allocate NodeCoordinate\n");
        free(data);
        return NULL;
    }

    // RebarFiberのメモリ確保
    data->rebar_fiber = allocate_rebar_fiber(rebar_num);
    if (data->rebar_fiber->positions == NULL) {
        fprintf(stderr, "Failed to allocate RebarFiber\n");
        free(data->x->coordinate);
        free(data->y->coordinate);
        free(data->z->coordinate);
        free(data);
        return NULL;
    }

    // 初期化処理
    initialize_modeling_data(data);

    return data;
}


// modeling_data構造体の表示関数 ----------------------------------------------------------------------------

// JSONのインデント用のスペースを出力
void print_indent_md(int level) {
    for (int i = 0; i < level; i++) {
        printf("    "); // 4スペースのインデント
    }
}

// NodeCoordinateをJSON形式で出力
void print_node_coordinate(const char* name, NodeCoordinate* node, int indent) {
    
    print_indent_md(indent);
    printf("\"%s\": {\n", name);
    print_indent_md(indent + 1);
    printf("\"node_num\": %d,\n", node->node_num);
    print_indent_md(indent + 1);
    if (node->coordinate == NULL) {
        // 解放済みの場合のメッセージ
        printf("\"coordinate\": null (freed)\n");
    } else {
        printf("\"coordinate\": [");
        for (int i = 0; i < node->node_num; i++) {
            printf("%lf", node->coordinate[i]);
            if (i < node->node_num - 1) printf(", ");
        }
        printf("]\n");
    }
    print_indent_md(indent);
    printf("}\n");
}

// NodeElementをJSON形式で出力
void print_node_element(NodeElement* elem, int indent) {
    if(indent > 0) {
        print_indent_md(indent);
    } else {

    }
    printf("{ \"node\": %d, \"element\": %d }", elem->node, elem->element);
}

// column_hexaを表示
void print_column_hexa(const char* name, ColumnHexa* column, int indent) {
    print_indent_md(indent);
    printf("\"%s\": {\n", name);
    print_indent_md(indent + 1);
    printf("\"increment\": [\n");
    for(int i = 0; i < 3; i++) {
        print_node_element(&column->increment[i], indent + 2);
        if (i < 2) printf(",\n");
    }
    printf("\n");
    print_indent_md(indent + 1);
    printf("],\n");
    print_indent_md(indent + 1);
    printf("\"occupied_indices\": \n");
    print_node_element(&column->occupied_indices, indent + 2);
    printf(",\n");
    print_indent_md(indent + 1);
    printf("\"head\": \n");
    print_node_element(&column->head, indent + 2);
    printf("\n");
    print_indent_md(indent);
    printf("},\n");
}

// RebarFiberをJSON形式で出力
void print_rebar_fiber(const char* name, RebarFiber* rebar, int indent) {
    print_indent_md(indent);
    printf("\"%s\": {\n", name);
    print_indent_md(indent + 1);
    printf("\"rebar_num\": %d,\n", rebar->rebar_num);

    print_indent_md(indent + 1);
    printf("\"positions\": [\n");
    for (int i = 0; i < rebar->rebar_num; i++) {
        print_indent_md(indent + 2);
        printf("{ \"x\": %d, \"y\": %d }", rebar->positions[i].x, rebar->positions[i].y);
        if (i < rebar->rebar_num - 1) printf(",\n");
    }
    printf("\n");
    print_indent_md(indent + 1);
    printf("],\n");

    print_indent_md(indent + 1);
    printf("\"increment\": \n");
    print_node_element(&rebar->increment, indent + 2); printf(",\n");
    print_indent_md(indent + 1);
    printf("\"occupied_indices\": \n");
    print_node_element(&rebar->occupied_indices, indent + 2); printf(",\n");
    print_indent_md(indent + 1);
    printf("\"occupied_indices_single\": \n");
    print_node_element(&rebar->occupied_indices_single, indent + 2); printf(",\n");
    print_indent_md(indent + 1);
    printf("\"head\": \n");
    print_node_element(&rebar->head, indent + 2);
    printf("\n");

    print_indent_md(indent);
    printf("},\n");
}

// RebarLine
void print_rebar_line(const char* name, RebarLine* rebar_line, int indent) {
    print_indent_md(indent);
    printf("\"%s\": {\n", name);
    print_indent_md(indent + 1);
    printf("\"increment\": ");
    print_node_element(&rebar_line->increment, 0); printf(",\n");
    print_indent_md(indent + 1);
    printf("\"occupied_indices\": ");
    print_node_element(&rebar_line->occupied_indices, 0); printf(",\n");
    print_indent_md(indent + 1);
    printf("\"occupied_indices_single\": ");
    print_node_element(&rebar_line->occupied_indices_single, 0); printf(",\n");
    print_indent_md(indent + 1);
    printf("\"head\": ");
    print_node_element(&rebar_line->head, 0); printf(",\n");
    print_indent_md(indent);
    printf("},\n");
}

// JointQuad
void print_joint_quad(const char* name, JointQuad* joint_quad, int indent) {
    print_indent_md(indent);
    printf("\"%s\": {\n", name);

    print_indent_md(indent + 1);
    printf("increment: [");
    for(int i = 0; i < 3; i++) {
        printf("%d", joint_quad->increment_element[i]);
        if (i < 2) printf(" ,");
    }
    printf("],\n");
    print_indent_md(indent + 1);
    printf("\"occupied_indices\": ");
    print_node_element(&joint_quad->occupied_indices, 0); printf(",\n");
    print_indent_md(indent + 1);
    printf("\"head\": ");
    print_node_element(&joint_quad->head, 0); printf(",\n");
    print_indent_md(indent);
    printf("}\n");
}

// JointFilm
void print_joint_film(const char* name, JointFilm* joint_film, int indent) {
    print_indent_md(indent);
    printf("\"%s\": {\n", name);

    print_indent_md(indent + 1);
    printf("occupied_indices: %d\n", joint_film->occupied_indices);
    print_indent_md(indent + 1);
    printf("head: %d\n", joint_film->head);

    print_indent_md(indent);
    printf("}\n");
}

// beam
void print_beam(const char* name, BeamHexaQuad* beam, int indent) {
    print_indent_md(indent);
    printf("\"%s\": {\n", name);

    print_indent_md(indent + 1);
    printf("\"increment\": [\n");
    for(int i = 0; i < 3; i++) {
        print_node_element(&beam->increment[i], indent + 2);
        if (i < 2) printf(",\n");
    }
    printf("\n");
    print_indent_md(indent + 1);
    printf("\"head\": ");
    print_node_element(&beam->head, 0);
    printf("\n");
    print_indent_md(indent);
    printf("}\n");
}

// ModelingDataをJSON形式で出力
void print_modeling_data(ModelingData* data) {
    printf("\n-------------------------------- modeling_data --------------------------------\n");
    printf("{\n");

    // NodeCoordinateの出力
    print_node_coordinate("x", data->x, 1); printf(",\n");
    print_node_coordinate("y", data->y, 1); printf(",\n");
    print_node_coordinate("z", data->z, 1); printf(",\n");

    // Boundary Indexの出力
    print_indent_md(1);
    printf("\"boundary_index\": [");
    for (int i = 0; i < BOUNDARY_X_MAX + BOUNDARY_Y_MAX + BOUNDARY_Z_MAX; i++) {
        printf("%d", data->boundary_index[i]);
        if (i < BOUNDARY_X_MAX + BOUNDARY_Y_MAX + BOUNDARY_Z_MAX - 1) printf(", ");
    }
    printf("],\n");

    // column_hexa
    print_column_hexa("column_hexa", &data->column_hexa, 1);
    
    // RebarFiberの出力
    print_rebar_fiber("rebar_fiber", data->rebar_fiber, 1);

    // RebarLine
    print_rebar_line("rebar_line", &data->rebar_line, 1);

    // jointQuad
    print_joint_quad("joint_quad", &data->joint_quad, 1);

    // jointFilm
    print_joint_film("joint_film", &data->joint_film, 1);

    // beam
    print_beam("beam", &data->beam, 1);

    print_indent_md(0);
    printf("}\n");
}