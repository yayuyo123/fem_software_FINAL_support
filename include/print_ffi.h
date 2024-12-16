#ifndef PRINT_FFI_H
#define PRINT_FFI_H

#include<stdio.h>

void print_head_template(FILE *f, int last_step, int disp_node, char disp_dir, int load_node, char load_dir);

void print_NODE(FILE *f, int node, double coordinate_x, double coordinate_y, double coordinate_z);

void print_COPYNODE(FILE *f, int start, int end, int interval, double meshLen, int increment, int set, int dir);

void print_BEAM(FILE *f, int elmIndex, int nodeIndex, int nodePp, int typb);

void print_QUAD_increment(FILE *f, int elmIndex, int startNode, int node_pp[], int dir1, int dir2, int TYPQ);

void print_QUAD_node(FILE *f, int elmIndex, int node[], int typq);

void print_HEXA_node(FILE *f, int element_index, int node[], int typh);

void print_HEXA_increment(FILE *f, int EleIndex, int Node_S, const int node_increment[], int TYPH);

void print_LINE_node(FILE *f, int element_index, int node[]);

void print_LINE_increment(FILE *f, int elmIndex, int nodeIndex1, int nodeIndex3, int pp);

void print_FILM_node(FILE *f, int element_index, int face1[], int face2[], int typf);

void print_FILM_increment(FILE *f, int elmIndex, int face1, int face2, const int nodePp[], int dir1, int dir2, int typf);

void print_COPYELM(FILE *f, int elm_S, int elm_E, int elm_Inter, int elm_Inc, int node_Inc, int set);

void print_TYPH(FILE *f, int typh, int mat_index, char material);

void print_TYPB(FILE *f, int typb, int mats);

void print_TYPL(FILE *f, int typl, int matj, int axis);

void print_TYPQ(FILE *f, int typq, int mats);

void print_TYPF(FILE *f, int typf, int matj);

void print_AXIS(FILE *f, int axis);

void print_MATC(FILE *f, int matc);

void print_MATS(FILE *f, int mats);

void print_MATJ(FILE *f, int matj);

void print_REST(FILE *f, int s, int e, int i, int rc, int inc, int set);

void print_SUB1(FILE *f, int s, int e, int i, int dir, int master, int mDir);

void print_ETYP(FILE *f, int s, int e, int i, int type, int inc, int set);

void print_STEP(FILE *f, int step_num);

void print_FN(FILE *f, int start_node, int end_node, int interval, double disp, char direction);

void print_UE(FILE *f, int start_element, int end_element, int interval, double unit, char direction, int face);

void print_OUT(FILE *f, int start_step, int end_step, int interval);

#endif