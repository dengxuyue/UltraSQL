TOP_DIR=../../../..
SRC_DIR=$(TOP_DIR)/src

C_SRC=ext_lib.c
E_OBJ=ext_lib

include $(SRC_DIR)/build/config.mk
include $(TOP_DIR)/test/unit/build/unit.mk
L_FLAG+=-lncurses -L$(TOP_DIR)/bin/readline/lib -lreadline -lhistory

include $(SRC_DIR)/build/rule.mk
