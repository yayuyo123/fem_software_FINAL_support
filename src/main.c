#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#define LATEST_UPPDATE             "2024.06.03"
#define TEMPLATE_FILE_NAME         "template.txt"
#define LOADING_TEMPLATE_FILE_NAME "step_disp.txt"
#define OUT_FILE_NAME              "out.ffi"
#define OUT_NODE_ELM_FILE          "node-elm.txt"
#define OPTCHAR                    "jtlbfs:o:v"

/*ある格子点間で節点コピーする*/
void auto_copynode(FILE *f, int start, int end, int inter, int startPoint, int endPoint, const double length[], const int nodePp, int dir)
{
    int index = start;
    int delt = end - start;
    for(int i = startPoint, cnt = 0; i < endPoint; i += cnt)
    {
        cnt = count_consecutive(i, endPoint, length);
        if(cnt < 0)
        {
            printf("[ERROR] X\n");
            return ;
        }
        if(end == 0)
        {
            print_COPYNODE(f, index, 0, 0, length[i], nodePp, cnt, dir);
        }
        else if(end != 0)
        {
            print_COPYNODE(f, index, index + delt, inter, length[i], nodePp, cnt, dir);
        }
        index += cnt * nodePp;
    }
}

void out_template()
{
    FILE *f = fopen(TEMPLATE_FILE_NAME, "r");
    if(f == NULL)
    {
        f = fopen(TEMPLATE_FILE_NAME, "w");
        if(f != NULL)
        {
            fprintf(f,
                "Column : Span() Width() Depth() X_Center() Y_Center()\n"
                "Beam   : Span() Width() Depth() Y_Center() Z_Center()\n"
                "xBeam  : Width()\n"
                "Rebar 1: X() Y()\n"
                "Rebar 2: X() Y()\n"
                "Rebar 3: X() Y()\n"
                "--\n"
                "X_Mesh_Sizes(1,2,3)\n"
                "Y_Mesh_Sizes(1,2,3)\n"
                "Z_Mesh_Sizes(1,2,3)\n"
                "END\n");
            fclose(f);
        }
        else
        {
            printf("[ERROR] file cant open\n");
            return ;
        }
    }
    else
    {
        printf(TEMPLATE_FILE_NAME " already exist\n");
        fclose(f);
    }
    return ;
}

void out_loading_template()
{
    FILE *f = fopen(LOADING_TEMPLATE_FILE_NAME, "r");
    if(f == NULL)
    {
        f = fopen(LOADING_TEMPLATE_FILE_NAME, "w");
        if(f != NULL)
        {
            fprintf(f,
                "step(2,3,4)\n"
                "disp(5,-5,10)\n"
                "END\n");
            fclose(f);
        }
        else
        {
            printf("[ERROR] file cant open\n");
            return ;
        }
    }
    else
    {
        printf(LOADING_TEMPLATE_FILE_NAME " already exist\n");
        fclose(f);
    }
    return ;
}

void show_usage()
{
    printf(
        "Hi! I'm RCS modeler in FINAL. [frcs]\n\n"
        "Enter a command\n"
        "------------------------------------------\n"
        "| frcs [input file] [options] [argument] |\n"
        "------------------------------------------\n\n"
        "[options]\n"
        "-t      : out template\n"
        "-l      : out step-disp template\n"
        "-b      : beam -> roller , column -> pin\n"
        "-s [arg]: write loading step\n"
        "-f      : full model \n"
    );
}

void console_version()
{
    printf("last updated -> %s\n", LATEST_UPPDATE);
}

int main(int argc, char *argv[])
{
    /*
        command [options] [arguments]
    */
    int         optchar;
    int         optb = 0;
    int         optf = 0;
    const char *inputFile = NULL;
    const char *stepDispFile = NULL;

    if(argc < 2)
    {
        show_usage();
        return 1;
    }
    while((optchar = getopt(argc, argv, OPTCHAR)) != -1)
    {
        switch(optchar){
            case 'j':
                printf("option j\n");
                return 0;
            case 't': //テンプレ出力
                printf("option t\n");
                out_template();
                return 0;
            case 'l': //laoding step出力
                printf("option l\n");
                out_loading_template();
                return 0;
            case 'b': //beam 梁加力
                optb = 1;
                printf("option b\n");
                break;
            case 'f':
                printf("option f\n");
                optf = 1;
                break;
            case 's':
                stepDispFile = optarg;
                printf("option s\n"
                       "Loading step file is \"%s\"\n", stepDispFile);
                break;
            case 'v':
                printf("option v\n");
                console_version();
                return 0;
            default:
                show_usage();
                return 0;
        }
    }
    if(optind < argc){
        inputFile = argv[optind];
    }else{
        fprintf(stderr, "No input file specified\n");
        return 1;
    }
    if(inputFile != NULL){
        printf("Input file is \"%s\"\n", inputFile);
    }
    return 0;
}