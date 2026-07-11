CUR_DIR := .
BIN_DIR := $(CUR_DIR)/bin
SRC_DIR := $(CUR_DIR)/src
INC_DIR := $(CUR_DIR)/inc
OBJ_DIR := $(CUR_DIR)/obj
LIB_DIR := $(CUR_DIR)/lib

LIB_STATIC_DIR := $(LIB_DIR)/static
LIB_SHARE_DIR = $(LIB_DIR)/share

C_FLAGS := -I $(INC_DIR) -Wall -Wextra -Werror -g
CC	:= gcc


create_objs:
	gcc -I $(INC_DIR) -c $(SRC_DIR)/main.c -o $(OBJ_DIR)/main.o
	gcc -I $(INC_DIR) -c $(SRC_DIR)/helloWorld.c -o $(OBJ_DIR)/helloWorld.o


create_lib:
	gcc -shared $(OBJ_DIR)/helloWorld.o -o $(LIB_SHARE_DIR)/libhelloWorld.so

all: create_objs create_lib
	gcc -o $(BIN_DIR)/main $(OBJ_DIR)/main.o -L$(LIB_SHARE_DIR) -lhelloWorld

make clean:
	rm -rf $(OBJ_DIR)/*.o
	rm -rf $(LIB_SHARE_DIR)/*.so
	rm -rf $(BIN_DIR)/