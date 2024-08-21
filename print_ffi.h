#ifndef PRINT_FFI_H
#define PRINT_FFI_H

#include<stdio.h>

void print_NODE(FILE *f, int node, const double coordinates[]);

void print_COPYNODE(FILE *f, int start, int end, int interval, double meshLen, int increment, int set, int dir);

void print_BEAM(FILE *f, int elmIndex, int nodeIndex, int nodePp, int typb);

void print_QUAD(FILE *f, int elmIndex, int startNode, const int node_pp[], int dir1, int dir2, int TYPQ);

void print_HEXA(FILE *f, int EleIndex, int Node_S, const int nodePp[], int TYPH);

void print_LINE(FILE *f, int elmIndex, int nodeIndex1, int nodeIndex3, int pp);

void print_FILM(FILE *f, int elmIndex, int face1, int face2, const int nodePp[], int dir1, int dir2, int typf);

void print_COPYELM(FILE *f, int elm_S, int elm_E, int elm_Inter, int elm_Inc, int node_Inc, int set);

void print_join(FILE *f, int s1, int e1, int i1, int s2, int e2, int i2);

void print_rest(FILE *f, int s, int e, int i, int rc, int inc, int set);

void print_sub1(FILE *f, int s, int e, int i, int dir, int master, int mDir);

void print_etyp(FILE *f, int s, int e, int i, int type, int inc, int set);


#endif