#include <stdio.h>

#define DATA_NUMBER 1024
#define SET_DATA_MAX 24

struct GraphData
{
    int dataNum;
    char head[24];
    char data1[DATA_NUMBER][24];
    char data2[DATA_NUMBER][24];
};

struct GraphData graphDate[SET_DATA_MAX];

int load_fgg(const char *inputFileName){

    int set_data_num = 0;
    int result;
    char ch;

    FILE *file = fopen(inputFileName,"r");
    if(file == NULL){
        perror("File opening failed");
        return 0;
    }

    for(int j = 0; j <= SET_DATA_MAX; j++){

        result = fscanf(file, "%d", &graphDate[j].dataNum);
        if(result != 1){break;}

        fscanf(file, "%23s", graphDate[j].head);
        while((ch = fgetc(file)) != '\n'){}
        for(int i = 0; i <= graphDate[j].dataNum; i++){
            fscanf(file, "%s %s", graphDate[j].data1[i], graphDate[j].data2[i]);
        }
        while((ch = fgetc(file)) != '\n' && ch != EOF){}
        set_data_num++;

        /*
        printf("num : %4d head : %s\n", graphDate[j].dataNum, graphDate[j].head);
        for(int i = 0; i <= graphDate[j].dataNum; i++){
            printf("data1 : %s data2 : %s\n", graphDate[j].data1[i], graphDate[j].data2[i]);
        }
        */
    }

    fclose(file);
    return set_data_num;
}

void print_csv(const char *outputFileName, int setDataNum){
    
    FILE *outputFile = fopen(outputFileName, "w");
    if(outputFile == NULL){
        perror("File opening failed");
        return ;
    }

    for(int i = 0; i < setDataNum; i++){
        fprintf(outputFile, "%s,", graphDate[i].head);
    }
    fprintf(outputFile,"\n");

    for(int j = 0; j <= graphDate[0].dataNum; j++){
        fprintf(outputFile, "%s,", graphDate[0].data1[j]);
        for(int i = 0; i < setDataNum; i++){
            fprintf(outputFile, "%s,", graphDate[i].data2[j]);
        }
        fprintf(outputFile,"\n");
    }
    fclose(outputFile);
}

int main(){

    int set_data_num = load_fgg("data.fgg");
    if(set_data_num <= 0){
        return 1;
    }else if(set_data_num > SET_DATA_MAX){
        printf("cant reached end of fgg file\n""SET_DATA_MAX"" is smaller\n");
        return 1;
    }
    printf("%d\n", set_data_num);
    print_csv("out.txt", set_data_num);

    return 0;
}