TOP_DIR=../../../..
SRC_DIR=$(TOP_DIR)/src

C_SRC=myclient.c
E_OBJ=myclient

include $(SRC_DIR)/build/config.mk
include $(TOP_DIR)/test/unit/build/unit.mk

ifndef MY_HOME
MY_HOME=$(HOME)/local/mysql-cluster/7.1.19
endif

I_FLAG+=-I$(MY_HOME)/include/mysql
L_FLAG+=-lcliv2 -L$(MY_HOME)/lib/mysql -lmysqlclient
include $(SRC_DIR)/build/rule.mk
