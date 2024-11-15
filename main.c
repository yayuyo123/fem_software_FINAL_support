#include <stdio.h>
#include <string.h>

#define DATA_NUMBER 1024
#define SET_DATA_MAX 64

struct GraphData
{
    int dataNum;    // 1セットに含まれるデータの数
    char head[24]; // グラフファイルの2行目は先頭20文字なので十分
    char data1[DATA_NUMBER][24];
    char data2[DATA_NUMBER][24];
};

struct FileData
{
    int dataSetNum; // データのセット数
    struct GraphData graphDate[SET_DATA_MAX];
};

struct FileData fileData;


/**
 * ファイルを読み込んでGraphDataに格納
 * @param{*char} 入力するファイル名
 */
int load_fgg(const char *inputFileName){

    printf("in function load_fgg\n");

    /**
     * 入力ファイルの終端まで届いたか
     * 1: yes
     * 0: no
     */
    int reachedEOF = 0;
    int dataSetNum = 0;
    
    FILE *file = fopen(inputFileName,"r");
    if(file == NULL){
        perror("error file opening failed");
        return reachedEOF;
    }

    for(int j = 0; j < SET_DATA_MAX; j++){

        // 1行分の文字列を格納する変数
        char buffer[256];
        char ch;

        // ファイルの終端に届いたら
        if(fgets(buffer, sizeof(buffer), file) == NULL){
            printf("reached EOF\n");
            fileData.dataSetNum = dataSetNum;
            printf("dataSetNum = %d\n", fileData.dataSetNum);
            reachedEOF = 1;
            break;
        }else {
            // データセットの数をカウント
            dataSetNum++;
        }

        /**
         * 先頭にあるデータの数を格納する
         * 戻り値が1なら成功
         */ 
        const int result = sscanf(buffer, "%d", &fileData.graphDate[j].dataNum);
        if(result != 1){break;}

        // FINALヘルプによるとグラフファイルの2行目は先頭20文字
        fscanf(file, "%23s", fileData.graphDate[j].head);
        printf("%s\n",fileData.graphDate[j].head);
        // 改行文字まで読み込む
        while((ch = fgetc(file)) != '\n'){}
        for(int i = 0; i <= fileData.graphDate[j].dataNum; i++){
            fscanf(file, "%s %s", fileData.graphDate[j].data1[i], fileData.graphDate[j].data2[i]);
        }
        while((ch = fgetc(file)) != '\n' && ch != EOF){}
    }

    fclose(file);
    return reachedEOF;
}

/**
 * 
 */
void print_csv(const char *outputFileName){
    
    FILE *outputFile = fopen(outputFileName, "w");
    if(outputFile == NULL){
        perror("error file opening failed");
        return ;
    }

    // 始めの列
    fprintf(outputFile, ",");
    // データのタイトル
    for(int i = 0; i < fileData.dataSetNum; i++){
        fprintf(outputFile, "%s,", fileData.graphDate[i].head);
    }
    fprintf(outputFile,"\n");

    for(int j = 0; j <= fileData.graphDate[0].dataNum; j++){
        // 始めの列
        fprintf(outputFile, "%s,", fileData.graphDate[0].data1[j]);
        // データ列
        for(int i = 0; i < fileData.dataSetNum; i++){
            fprintf(outputFile, "%s,", fileData.graphDate[i].data2[j]);
        }
        fprintf(outputFile,"\n");
    }
    fclose(outputFile);
}

/**
 * 入力されたFINALのグラフファイルを各グラフごとに横に並べ直す。
 * @param{string} 入力するファイル名
 * @param{string} 出力するファイル名
 */
int main(int argc, char *argv[]){

    // 引数が足りない場合
    if(argc < 3){
        printf("error input file name\n");
        return 1;
    }

    char *inputFileName = argv[1];
    char *outputFileName = argv[2];
    printf("input filename is \"%s\" \n", inputFileName);
    printf("output filename is \"%s\" \n", outputFileName);

    /**
     * 入力ファイルの終端まで届いたか
     * 1: yes
     * 0: no
     */
    int reachedEOF = load_fgg(inputFileName);
    if(reachedEOF == 0){
        printf("error cant reached end of file\n");
        return 1;
    }

    print_csv(outputFileName);

    return 0;
}