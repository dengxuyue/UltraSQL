TOP_DIR=../../../..
SRC_DIR=$(TOP_DIR)/src

C_SRC=cmdhistory.c
E_OBJ=cmdhistory

include $(SRC_DIR)/build/config.mk
include $(TOP_DIR)/test/unit/build/unit.mk
L_FLAG+=-lncurses -lreadline -lhistory

include $(SRC_DIR)/build/rule.mk
