TOP_DIR=../../../..
SRC_DIR=$(TOP_DIR)/src

C_SRC=tdodbc.c
E_OBJ=tdodbc

include $(SRC_DIR)/build/config.mk
include $(TOP_DIR)/test/unit/build/unit.mk

ifndef ODBC_HOME
ODBC_HOME=/opt/teradata/client/ODBC_64
endif

I_FLAG+=-I$(ODBC_HOME)/include
L_FLAG+=-L$(ODBC_HOME) -lodbc

include $(SRC_DIR)/build/rule.mk

tdodbc.c: $(SRC_DIR)/interface/odbc.c
