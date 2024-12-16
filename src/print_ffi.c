#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "print_ffi.h"


/**
 * 指定した整数が指定した桁数以内であるかを判定する関数。
 * 
 * @param value 判定対象の整数値（正の数、負の数を含む）。
 * @param digits 許容する桁数（正の整数で指定）。0以下を指定すると false を返す。
 * @return 値が指定された桁数以内であれば true、そうでなければ false。
 * 
 * 桁数の計算には pow 関数を使用しており、digits 桁の最大値を 10^digits - 1 として計算。
 * 負の値については絶対値を使用して判定する。
 * 
 * 例:
 * - is_within_digits(12345, 5) => true
 * - is_within_digits(-99999, 5) => true
 * - is_within_digits(100000, 5) => false
 */
bool is_integer_within_digits(int value, int digits) {
    if (digits <= 0) {
        // 桁数が0以下の場合は無効（エラー）
        return false;
    }

    // 桁数に対応する最大値
    int max_value = (int)pow(10, digits) - 1;

    // 絶対値が許容範囲内であることを確認
    return abs(value) <= max_value;
}

/**
 * 入力ファイルに先頭の解析制御データを書き込む
 * 
 * @param f ファイルストリーム
 * @param last_step 実行する解析の最終ステップ
 * @param disp_node 変位をモニターする節点番号
 * @param disp_dir 変位をモニターする方向
 * @param load_node 荷重をモニターする節点番号
 * @param load_dir 荷重をモニターする方向
 */
int print_head_template(FILE *f, int last_step, int disp_node, char disp_dir, int load_node, char load_dir) {

    // ファイルポインタを確認
    if (f == NULL) {
        fprintf(stderr, "Error: Invalid file pointer\n");
        return EXIT_FAILURE;
    }

    // last_stepの確認
    char last_step_str[6] = " ";
    if (last_step < 0) {
        // 0未満の場合はエラー
        perror("Error: 'last_step' < 0\n");
        return EXIT_FAILURE;
    } else if (last_step == 0) {
        // 0の場合は空白を書き込む
    } else if (last_step > 0) {
        // 0より大きい場合、5桁以下であることを確認
        if(is_integer_within_digits(last_step, 5) == true) {
            sprintf(last_step_str, "%d", last_step);
        } else {
            perror("Error: 'last_step' exceeds 5 digits (greater than 99999)\n");
            return EXIT_FAILURE;
        }
    }

    // disp_node
    char disp_node_str[6] = " ";
    if(disp_node < 0) {
        // 0未満の場合はエラー
        perror("Error: 'disp_node' < 0\n");
        return EXIT_FAILURE;
    } else if (disp_node == 0) {
    } else if (disp_node > 0) {
        // 5桁以内であることを確認
        if(is_integer_within_digits(disp_node, 5) == true) {
            sprintf(disp_node_str, "%d", disp_node);
        } else {
            perror("Error: 'disp_node' exceeds 5 digits (greater than 99999)\n");
            return EXIT_FAILURE;
        }
    }

    // load_node
    char load_node_str[6] = " ";
    if (load_node < 0) {
        // 0未満の場合はエラー
        perror("Error: 'load_node' < 0\n");
        return EXIT_FAILURE;
    } else if (load_node == 0) {
        
    } else if(load_node > 0) {
        if(is_integer_within_digits(load_node, 5) == true) {
            sprintf(load_node_str, "%d", load_node);
        } else {
            perror("Error: 'load_node' exceeds 5 digits (greater than 99999)\n");
            return EXIT_FAILURE;
        }
        
    }

    // disp_dir の確認
    int disp_direction_int = 0;
    if (disp_dir == 'x' || disp_dir == 'X') {
        disp_direction_int = 1;
    } else if (disp_dir == 'y' || disp_dir == 'Y') {
        disp_direction_int = 2;
    } else if (disp_dir == 'z' || disp_dir == 'Z') {
        disp_direction_int = 3;
    } else {
        fprintf(stderr, "Error: 'load_dir' must be 'x', 'y', or 'z'\n");
        return EXIT_FAILURE;
    }

    // load_dir の確認
    int load_direction_int = 0;
    if (load_dir == 'x' || load_dir == 'X') {
        load_direction_int = 1;
    } else if (load_dir == 'y' || load_dir == 'Y') {
        load_direction_int = 2;
    } else if (load_dir == 'z' || load_dir == 'Z') {
        load_direction_int = 3;
    } else {
        fprintf(stderr, "Error: 'load_dir' must be 'x', 'y', or 'z'\n");
        return EXIT_FAILURE;
    }

    // ファイルへ書き込み - 戻り値は書き込んだ文字数
    int result = fprintf(
        f,
        "-------------------< FINAL version 11  Input data >---------------------\n"
        "TITL :\n"
        "EXEC :STEP (    1)-->(%5s)  ELASTIC=( ) CHECK=(1) POST=(1) RESTART=( )\n"
        "LIST :ECHO=(0)  MODEL=(1)  RESULTS=(1)  MESSAGE=(2)  WARNING=(2)  (0:NO)\n"
        "FILE :CONV=(2)  GRAPH=(2)  MONITOR=(2)  HISTORY=(1)  ELEMENT=(0)  (0:NO)\n"
        "DISP :DISPLACEMENT MONITOR NODE NO.(%5s)  DIR=(%1d)    FACTOR=\n"
        "LOAD :APPLIED LOAD MONITOR NODE NO.(%5s)  DIR=(%1d)    FACTOR=\n"
        "UNIT :STRESS=(3) (1:kgf/cm**2  2:tf/m**2  3:N/mm**2=MPa)\n\n",
        last_step_str, disp_node_str, disp_direction_int, load_node_str, load_direction_int
    );

    // 書き込んだ文字数が0未満であればエラーを返す
    if(result < 0) {
        perror("Error writing to file\n");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}


// データ入力用関数 -----------------------------------------------------------------------------------------
void print_NODE(FILE *f, int node, double coordinate_x, double coordinate_y, double coordinate_z) {
    fprintf(f, "NODE :(%5d)  X=%-10.2fY=%-10.2fZ=%-10.2fRC=(000000)\n", node, coordinate_x, coordinate_y, coordinate_z);
}

/**
 * dirは[0,1,2] -> [x,y,z]
 */
void print_COPYNODE(FILE *f, int start, int end, int interval, double meshLen, int increment, int set, int dir) 
{
    char _end[6]      = " ";
    char _interval[6] = " ";

    if (end != 0)
    {
        sprintf(_end, "%d", end);
    }

    if (interval != 0)
    {
        sprintf(_interval, "%d", interval);
    }

    if (set <= 0)
     {} 
    else 
    {
        switch (dir) 
        {
            case 0  : fprintf(f, "COPY :NODE  S(%5d)-E(%5s)-I(%5s)  DX=%-9.2lfINC(%5d)-SET(%4d)\n", start, _end, _interval, meshLen, increment, set); break;
            case 1  : fprintf(f, "COPY :NODE  S(%5d)-E(%5s)-I(%5s)  DY=%-9.2lfINC(%5d)-SET(%4d)\n", start, _end, _interval, meshLen, increment, set); break;
            case 2  : fprintf(f, "COPY :NODE  S(%5d)-E(%5s)-I(%5s)  DZ=%-9.2lfINC(%5d)-SET(%4d)\n", start, _end, _interval, meshLen, increment, set); break;
            default : printf("[ERROR] CopyNode\n");
        }
    }
}

void print_BEAM(FILE *f, int elmIndex, int nodeIndex, int nodePp, int typb)
{
    fprintf(f, "BEAM :(%5d)(%5d:%5d) TYPB(%3d)  Y-NODE(     )\n", elmIndex, nodeIndex, nodeIndex + nodePp, typb);
}

void print_QUAD_increment(FILE *f, int elmIndex, int startNode, int node_pp[], int dir1, int dir2, int TYPQ)
{
    fprintf(f, "QUAD :(%5d)(%5d:%5d:%5d:%5d) TYPQ(%3d)\n", elmIndex, startNode, startNode + node_pp[dir1], startNode + node_pp[dir1] + node_pp[dir2], startNode + node_pp[dir2], TYPQ);
}

void print_QUAD_node(FILE *f, int elmIndex, int node[], int typq)
{
    fprintf(f, "QUAD :(%5d)(%5d:%5d:%5d:%5d) TYPQ(%3d)\n", elmIndex, node[0], node[1], node[2], node[3], typq);
}

/**
 * nodeは節点番号を格納した配列、要素数は8。
 */
void print_HEXA_node(FILE *f, int element_index, int node[], int typh)
{
    fprintf(f, "HEXA :(%5d)(%5d:%5d:%5d:%5d:%5d:%5d:%5d:%5d) TYPH(%3d)\n",
        element_index, node[0], node[1], node[2], node[3], node[4], node[5], node[6], node[7], typh);
}

void print_HEXA_increment(FILE *f, int EleIndex, int Node_S, const int node_increment[], int TYPH)
{
    fprintf(f, "HEXA :(%5d)(%5d:%5d:%5d:%5d:%5d:%5d:%5d:%5d) TYPH(%3d)\n",
        EleIndex, Node_S, Node_S + node_increment[0], Node_S + node_increment[0] + node_increment[1], Node_S + node_increment[1], Node_S + node_increment[2], Node_S + node_increment[0] + node_increment[2], Node_S + node_increment[0] + node_increment[1] + node_increment[2], Node_S + node_increment[1] + node_increment[2], TYPH);
}

/**
 * nodeは節点番号を格納した配列
 */
void print_LINE_node(FILE *f, int element_index, int node[])
{
    fprintf(f, "LINE :(%5d)(%5d:%5d:%5d:%5d) TYPL(  1)\n", element_index, node[0], node[1], node[2], node[3]);
}

void print_LINE_increment(FILE *f, int elmIndex, int nodeIndex1, int nodeIndex3, int pp)
{
    fprintf(f, "LINE :(%5d)(%5d:%5d:%5d:%5d) TYPL(  1)\n", elmIndex, nodeIndex1, nodeIndex1 + pp, nodeIndex3, nodeIndex3 + pp);
}

void print_FILM_node(FILE *f, int element_index, int face1[], int face2[], int typf)
{
    fprintf(f, "FILM :(%5d)(%5d:%5d:%5d:%5d:%5d:%5d:%5d:%5d) TYPF(%3d)\n", element_index, face1[0], face1[1], face1[2], face1[3], face2[0], face2[1], face2[2], face2[3], typf);
}

void print_FILM_increment(FILE *f, int elmIndex, int face1, int face2, const int nodePp[], int dir1, int dir2, int typf)
{
    fprintf(f, "FILM :(%5d)(%5d:%5d:%5d:%5d:%5d:%5d:%5d:%5d) TYPF(%3d)\n", elmIndex, face1, face1 + nodePp[dir1], face1 + nodePp[dir1] + nodePp[dir2], face1 + nodePp[dir2], face2, face2 + nodePp[dir1], face2 + nodePp[dir1] + nodePp[dir2], face2 + nodePp[dir2], typf);
}

void print_COPYELM(FILE *f, int elm_S, int elm_E, int elm_Inter, int elm_Inc, int node_Inc, int set)
{
    char end[6] = " ";
    char interval[6] = " ";
    if (elm_E != 0)
    {
        sprintf(end, "%d", elm_E);
    }
    if (elm_Inter != 0)
    {
        sprintf(interval, "%d", elm_Inter);
    }
    fprintf(f, "COPY :ELM  S(%5d)-E(%5s)-I(%5s)   INC(%5d)-NINC(%5d)-SET(%4d)\n", elm_S, end, interval, elm_Inc, node_Inc, set);
}

/**
 * @param f 
 * @param typh 六面体要素番号
 * @param mat_index 材料番号
 * @param material c:コンクリート、s:鋼材
 */
void print_TYPH(FILE *f, int typh, int mat_index, char material) {
    if(material == 'c' || material == 'C') {
        fprintf(f, "TYPH :(%3d)  MATC(%3d)  AXIS(  0)\n", typh, mat_index);
    } else if(material == 's' || material == 'S') {
        fprintf(f, "TYPH :(%3d)  MATS(%3d)  AXIS(  0)\n", typh, mat_index);
    }
}

void print_TYPB(FILE *f, int typb, int mats)
{
    fprintf(f, "TYPB :(%3d)  MATS(%3d)  AXIS(  0)  AREA=1       LY=        LZ=        :\n", typb, mats);
}

void print_TYPL(FILE *f, int typl, int matj, int axis)
{
    fprintf(f, "TYPL :(%3d)  MATJ(%3d)  AXIS(%3d)  THICKNESS=1.0      Z=(1) (1:N  2:S)\n", typl, matj, axis);
}

void print_TYPQ(FILE *f, int typq, int mats)
{
    fprintf(f, "TYPQ :(%3d)  MATS(%3d)  AXIS(  0)  THICKNESS=1.0     P-STRAIN=(0) (0:NO)\n", typq, mats);
}

void print_TYPF(FILE *f, int typf, int matj)
{
    fprintf(f, "TYPF :(%3d)  MATJ(%3d)  AXIS(  0)\n", typf, matj);
}

void print_AXIS(FILE *f, int axis)
{
    fprintf(f, "AXIS :(%3d)  TYPE=(1) (1:GLOBAL 2:ELEMENT 3:INPUT 4:CYLINDER 5:SPHERE)\n", axis);
}

void print_MATC(FILE *f, int matc)
{
    fprintf(f, "MATC :(%3d)  EC=2      (E+4) PR=0.2   FC=30     FT=      ALP=      (E-5)\n", matc);
}

void print_MATS(FILE *f, int mats)
{
    fprintf(f, "MATS :(%3d)  ES=2      (E+5) PR=0.3   SY=300    HR=0.01  ALP=      (E-5)\n", mats);
}

void print_MATJ(FILE *f, int matj)
{
    fprintf(f, "MATJ :(%3d)  TYPE=(4) (1:CRACK  2:BOND  3:GENERIC  4:RIGID  5:DASHPOT)\n", matj);
}


void print_REST(FILE *f, int s, int e, int i, int rc, int inc, int set)
{
    fprintf(f, "REST :NODE  S(%5d)-E(%5d)-I(%5d)  RC=(%03d000) INC(%5d)-SET(%4d)\n", s, e, i, rc, inc, set);
}

void print_SUB1(FILE *f, int s, int e, int i, int dir, int master, int mDir)
{
    fprintf(f, "SUB1 :NODE  S(%5d)-E(%5d)-I(%5d)-D(%1d)  M(%5d)-D(%1d)  F=1\n", s, e, i, dir, master, mDir);
}

void print_ETYP(FILE *f, int s, int e, int i, int type, int inc, int set)
{
    fprintf(f, "ETYP :ELM  S(%5d)-E(%5d)-I(%5d)  TYPE(%3d)  INC(%5d)-SET(%4d)\n", s, e, i, type, inc, set);
}

void print_STEP(FILE *f, int step_num)
{
    fprintf(f, "STEP :UP TO NO.(%5d)   MAXIMUM LOAD INCREMENT=         CREEP=(0)(0:NO)\n", step_num);
}

/**
 * 未完成
 */
void print_FN(FILE *f, int start_node, int end_node, int interval, double disp, char direction)
{
    char end_node_str[6] = " ";
    char interval_str[6] = " ";
    if (end_node != 0)
    {
        sprintf(end_node_str, "%d", end_node);
    }
    if (interval != 0)
    {
        sprintf(interval_str, "%d", interval);
    }

    int direction_int = 0;
    if (direction == 'x' || direction == 'X') {
        direction_int = 1;
    } else if (direction == 'y' || direction == 'Y') {
        direction_int = 2;
    } else if (direction == 'z' || direction == 'Z') {
        direction_int = 3;
    } else {
        direction_int = 0;
    }

    fprintf(f,"  FN :NODE  S(%5d)-E(%5s)-I(%5s)     DISP=%-9.2fDIR(%1d)\n", start_node, end_node_str, interval_str, disp, direction_int);
}

/**
 * 六面体要素限定で要素一様荷重を与える。
 * 
 * @param f
 * @param start_element
 * @param end_element
 * @param interval
 * @param unit　単位面積当りの荷重
 * @param direction　荷重の方向
 * @param face 1: 下面, 2: 上面
 */
void print_UE(FILE *f, int start_element, int end_element, int interval, double unit, char direction, int face) {
    int direction_int = 0;
    if (direction == 'x' || direction == 'X') {
        direction_int = 1;
    } else if (direction == 'y' || direction == 'Y') {
        direction_int = 2;
    } else if (direction == 'z' || direction == 'Z') {
        direction_int = 3;
    } else {
        direction_int = 0;
    }

    fprintf(f,"  UE :ELM   S(%5d)-E(%5d)-I(%5d)     UNIT=%-9.2fDIR(%1d)  FACE(%1d)\n", start_element, end_element, interval, unit, direction_int, face);
}

void print_OUT(FILE *f, int start_step, int end_step, int interval) {
    char end_node_str[6] = " ";
    char interval_str[6] = " ";
    if (end_step != 0)
    {
        sprintf(end_node_str, "%d", end_step);
    }
    if (interval != 0)
    {
        sprintf(interval_str, "%d", interval);
    }

    fprintf(f," OUT :STEP  S(%5d)-E(%5s)-I(%5s) LEVEL=(3) (1:RESULT 2:POST 3:1+2)\n", start_step, end_node_str, interval_str);
}