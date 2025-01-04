# コンパイラとフラグ
CC = gcc
CFLAGS = -Wall -I./include

# 出力ディレクトリ
OBJ_DIR = ./obj
BIN_DIR = ./bin

# ソースディレクトリ
SRC_DIR = ./src
TEST_SRC = ./test

# ソースファイル
# function.c json_parser.c ...
SOURCES = $(wildcard $(SRC_DIR)/*.c)

# オブジェクトファイルのパス
# .c -> .oに変換
OBJECTS = $(addprefix $(OBJ_DIR)/, $(notdir $(SOURCES:.c=.o)))

# デフォルトターゲット
all: clean main test

# メインターゲット
main: ./cli/main.c $(OBJECTS)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/$@ ./cli/main.c $(OBJECTS)

# テストプログラム
test: clean $(OBJECTS)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/test ./test/main.c ./test/test.c $(OBJECTS)

# パターンルール: ソースファイルをオブジェクトファイルに変換
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# クリーンアップ
# bin, objディレクトリを削除後、作成する。
clean:
	if exist $(notdir $(OBJ_DIR)) rmdir /s /q $(notdir $(OBJ_DIR))
	if exist $(notdir $(BIN_DIR)) rmdir /s /q $(notdir $(BIN_DIR))
	mkdir $(notdir $(OBJ_DIR))
	mkdir $(notdir $(BIN_DIR))