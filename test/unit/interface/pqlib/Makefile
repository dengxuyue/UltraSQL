TOP_DIR=../../../..
SRC_DIR=$(TOP_DIR)/src

C_SRC=pqlib.c
E_OBJ=pqlib

include $(SRC_DIR)/build/config.mk
include $(TOP_DIR)/test/unit/build/unit.mk

ifndef PQ_HOME
PQ_HOME=$(HOME)/local/pgsql/9.5
endif

I_FLAG+=-I$(PQ_HOME)/include
L_FLAG+=-L$(PQ_HOME)/lib -lpq
include $(SRC_DIR)/build/rule.mk
