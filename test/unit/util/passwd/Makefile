TOP_DIR=../../../..
SRC_DIR=$(TOP_DIR)/src

C_SRC=passwd.c
E_OBJ=passwd

include $(SRC_DIR)/build/config.mk
include $(TOP_DIR)/test/unit/build/unit.mk
L_FLAG+=-lncurses -L$(TOP_DIR)/bin/readline/lib -lreadline -lhistory

include $(SRC_DIR)/build/rule.mk
