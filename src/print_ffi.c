#include <stdio.h>
#include "print_ffi.h"

void print_NODE(FILE *f, int node, const double coordinates[]) 
{
    fprintf(f, "NODE :(%5d)  X=%-10.2fY=%-10.2fZ=%-10.2fRC=(000000)\n", node, coordinates[0], coordinates[1], coordinates[2]);
}

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

void print_QUAD(FILE *f, int elmIndex, int startNode, const int node_pp[], int dir1, int dir2, int TYPQ)
{
    fprintf(f, "QUAD :(%5d)(%5d:%5d:%5d:%5d) TYPQ(%3d)\n", elmIndex, startNode, startNode + node_pp[dir1], startNode + node_pp[dir1] + node_pp[dir2], startNode + node_pp[dir2], TYPQ);
}

void print_HEXA(FILE *f, int EleIndex, int Node_S, const int nodePp[], int TYPH)
{
    fprintf(f, "HEXA :(%5d)(%5d:%5d:%5d:%5d:%5d:%5d:%5d:%5d) TYPH(%3d)\n", EleIndex, Node_S, Node_S + nodePp[0], Node_S + nodePp[0] + nodePp[1], Node_S + nodePp[1], Node_S + nodePp[2], Node_S + nodePp[0] + nodePp[2], Node_S + nodePp[0] + nodePp[1] + nodePp[2], Node_S + nodePp[1] + nodePp[2], TYPH);
}

void print_LINE(FILE *f, int elmIndex, int nodeIndex1, int nodeIndex3, int pp)
{
    fprintf(f, "LINE :(%5d)(%5d:%5d:%5d:%5d) TYPL(  1)\n", elmIndex, nodeIndex1, nodeIndex1 + pp, nodeIndex3, nodeIndex3 + pp);
}

void print_FILM(FILE *f, int elmIndex, int face1, int face2, const int nodePp[], int dir1, int dir2, int typf)
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

void print_join(FILE *f, int s1, int e1, int i1, int s2, int e2, int i2)
{
    fprintf(f, "JOIN :NODE  S(%5d)-E(%5d)-I(%5d)  WITH  S(%5d)-E(%5d)-I(%5d)\n", s1, e1, i1, s2, e2, i2);
}

void print_rest(FILE *f, int s, int e, int i, int rc, int inc, int set)
{
    fprintf(f, "REST :NODE  S(%5d)-E(%5d)-I(%5d)  RC=(%03d000) INC(%5d)-SET(%4d)\n", s, e, i, rc, inc, set);
}

void print_sub1(FILE *f, int s, int e, int i, int dir, int master, int mDir)
{
    fprintf(f, "SUB1 :NODE  S(%5d)-E(%5d)-I(%5d)-D(%1d)  M(%5d)-D(%1d)  F=1\n", s, e, i, dir, master, mDir);
}

void print_etyp(FILE *f, int s, int e, int i, int type, int inc, int set)
{
    fprintf(f, "ETYP :ELM  S(%5d)-E(%5d)-I(%5d)  TYPE(%3d)  INC(%5d)-SET(%4d)\n", s, e, i, type, inc, set);
}
