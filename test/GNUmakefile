CFLAGS = -Wall -I../include
CC = gcc

# 出力フォルダを指定
OBJ_DIR = ../obj

# OBJ_DIRが存在しない場合に作成
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# json_parserをテストするプログラムのビルド
test_json_parser: $(OBJ_DIR) $(OBJ_DIR)/test_json_parser.o $(OBJ_DIR)/json_parser.o $(OBJ_DIR)/parson.o
	$(CC) $(CFLAGS) -o test_json_parser $(OBJ_DIR)/test_json_parser.o $(OBJ_DIR)/json_parser.o $(OBJ_DIR)/parson.o

# test_json_parser.o のコンパイル
$(OBJ_DIR)/test_json_parser.o: test_json_parser.c
	$(CC) $(CFLAGS) -c test_json_parser.c -o $(OBJ_DIR)/test_json_parser.o

# modeling_rcsをテストするプログラムのビルド
test_modeling_rcs: $(OBJ_DIR) $(OBJ_DIR)/test_modeling_rcs.o $(OBJ_DIR)/modeling_rcs.o $(OBJ_DIR)/json_parser.o $(OBJ_DIR)/parson.o $(OBJ_DIR)/function.o $(OBJ_DIR)/print_ffi.o
	$(CC) $(CFLAGS) -o ../final_test/test_modeling_rcs $(OBJ_DIR)/test_modeling_rcs.o $(OBJ_DIR)/modeling_rcs.o $(OBJ_DIR)/json_parser.o $(OBJ_DIR)/parson.o $(OBJ_DIR)/function.o $(OBJ_DIR)/print_ffi.o

# test_modeling_rcs.o のコンパイル
$(OBJ_DIR)/test_modeling_rcs.o: test_modeling_rcs.c
	$(CC) $(CFLAGS) -c test_modeling_rcs.c -o $(OBJ_DIR)/test_modeling_rcs.o

# modeling_rcs.o のコンパイル
$(OBJ_DIR)/modeling_rcs.o: ../src/modeling_rcs.c
	$(CC) $(CFLAGS) -c ../src/modeling_rcs.c -o $(OBJ_DIR)/modeling_rcs.o

# json_parser.o のコンパイル
$(OBJ_DIR)/json_parser.o: ../src/json_parser.c
	$(CC) $(CFLAGS) -c ../src/json_parser.c -o $(OBJ_DIR)/json_parser.o

# parson.o のコンパイル
$(OBJ_DIR)/parson.o: ../src/parson.c
	$(CC) $(CFLAGS) -c ../src/parson.c -o $(OBJ_DIR)/parson.o

# function.o のコンパイル
$(OBJ_DIR)/function.o: ../src/function.c
	$(CC) $(CFLAGS) -c ../src/function.c -o $(OBJ_DIR)/function.o

# print_ffi.o
$(OBJ_DIR)/print_ffi.o: ../src/print_ffi.c
	$(CC) $(CFLAGS) -c ../src/print_ffi.c -o $(OBJ_DIR)/print_ffi.o

