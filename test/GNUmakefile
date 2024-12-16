# コンパイラとフラグ
CC = gcc
CFLAGS = -Wall -I../include

# 出力ディレクトリ
OBJ_DIR = ../obj
BIN_DIR = ../final_test

# ソースファイル
SRC = ../src
TEST_SRC = .

# 対象ファイル
OBJS = $(OBJ_DIR)/json_parser.o $(OBJ_DIR)/parson.o $(OBJ_DIR)/function.o $(OBJ_DIR)/print_ffi.o
TEST_OBJS = $(OBJ_DIR)/test_json_parser.o $(OBJ_DIR)/test_modeling_rcs.o $(OBJ_DIR)/test_print_ffi.o
MODEL_OBJS = $(OBJ_DIR)/modeling_rcs.o

# ターゲット実行ファイル
BIN_TEST_JSON = test_json_parser
BIN_TEST_MODELING = $(BIN_DIR)/test_modeling_rcs
BIN_TEST_PRINT = test_print_ffi

# デフォルトターゲット
all: $(BIN_TEST_JSON) $(BIN_TEST_MODELING) $(BIN_TEST_PRINT)

# ディレクトリを作成
$(OBJ_DIR) $(BIN_DIR):
	mkdir -p $@

# 実行ファイル: test_json_parser
$(BIN_TEST_JSON): $(OBJ_DIR) $(OBJS) $(OBJ_DIR)/test_json_parser.o
	$(CC) $(CFLAGS) -o $@ $(OBJ_DIR)/test_json_parser.o $(OBJS)

# 実行ファイル: test_modeling_rcs
$(BIN_TEST_MODELING): $(OBJ_DIR) $(BIN_DIR) $(OBJS) $(MODEL_OBJS) $(OBJ_DIR)/test_modeling_rcs.o
	$(CC) $(CFLAGS) -o $@ $(OBJ_DIR)/test_modeling_rcs.o $(MODEL_OBJS) $(OBJS)

# 実行ファイル: test_print_ffi
$(BIN_TEST_PRINT): $(OBJ_DIR) $(OBJS) $(OBJ_DIR)/test_print_ffi.o
	$(CC) $(CFLAGS) -o $@ $(OBJ_DIR)/test_print_ffi.o $(OBJS)

# パターンルール: ソースファイルをオブジェクトファイルに変換
$(OBJ_DIR)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/test_%.o: $(TEST_SRC)/test_%.c
	$(CC) $(CFLAGS) -c $< -o $@

# クリーンアップ
clean:
	rm -rf $(OBJ_DIR) $(BIN_TEST_JSON) $(BIN_TEST_MODELING) $(BIN_TEST_PRINT)
