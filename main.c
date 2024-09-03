#include <stdio.h>
#include <string.h>

#define DATA_NUMBER 1024
#define SET_DATA_MAX 64

struct GraphData
{
    int dataNum;
    char head[24];
    char data1[DATA_NUMBER][24];
    char data2[DATA_NUMBER][24];
};

struct GraphData graphDate[SET_DATA_MAX];

/*
 *
 */


int load_fgg(const char *inputFileName){

    printf("in load_fgg\n");

    int reachedEOF = 0; //1:Yes, 0:No
    int result;
    char ch;

    FILE *file = fopen(inputFileName,"r");
    if(file == NULL){
        perror("File opening failed");
        return reachedEOF;
    }

    for(int j = 0; j < SET_DATA_MAX; j++){

        char buffer[256];

        if(fgets(buffer, sizeof(buffer), file) == NULL){
            printf("reached EOF\n");
            reachedEOF = 1;
            break;
        }

        result = sscanf(buffer, "%d", &graphDate[j].dataNum);
        if(result != 1){break;}

        fscanf(file, "%23s", graphDate[j].head);
        printf("%s\n",graphDate[j].head);
        while((ch = fgetc(file)) != '\n'){/*printf("!\n");*/}
        for(int i = 0; i <= graphDate[j].dataNum; i++){
            fscanf(file, "%s %s", graphDate[j].data1[i], graphDate[j].data2[i]);
        }
        while((ch = fgetc(file)) != '\n' && ch != EOF){}
    }

    fclose(file);
    return reachedEOF;
}

void print_csv(const char *outputFileName){
    
    printf("in print_csv\n");

    FILE *outputFile = fopen(outputFileName, "w");
    if(outputFile == NULL){
        perror("File opening failed");
        return ;
    }

    for(int i = 0; i < SET_DATA_MAX; i++){
        fprintf(outputFile, "%s,", graphDate[i].head);
    }
    fprintf(outputFile,"\n");

    for(int j = 0; j <= graphDate[0].dataNum; j++){
        fprintf(outputFile, "%s,", graphDate[0].data1[j]);
        for(int i = 0; i < SET_DATA_MAX; i++){
            fprintf(outputFile, "%s,", graphDate[i].data2[j]);
        }
        fprintf(outputFile,"\n");
    }
    fclose(outputFile);
}

int main(){

    char *inputFileName = "data.fgg";

    printf("%s\n",inputFileName);
    int reachedEOF = load_fgg(inputFileName);
    if(reachedEOF == 0){
        printf("catn reached end of file\n");
        return 1;
    }

    print_csv("out.txt");

    return 0;
}