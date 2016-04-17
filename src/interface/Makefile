TOP_DIR=../..
SRC_DIR=..

C_SRC=dbi.c
ifeq ($(MYSQL_SUPPORT),yes)
C_SRC+= myclient.c
endif
ifeq ($(POSTGRESQL_SUPPORT),yes)
C_SRC+= pqlib.c
endif
ifeq ($(TDCLI_SUPPORT),yes)
C_SRC+= tdcli.c
endif
ifeq ($(ODBC_SUPPORT),yes)
C_SRC+= odbc.c
endif


include $(SRC_DIR)/build/config.mk

I_FLAG+=-I$(PQ_HOME)/include
I_FLAG+=-I$(MY_HOME)/include
I_FLAG+=-I$(TDODBC)/include

all: $(O_OBJ)

include $(SRC_DIR)/build/rule.mk
