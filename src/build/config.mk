
CC=gcc
LD=gcc
LEX=flex
YACC=bison

C_FLAG=-g -DNO_CLIV2_ERROR_T

ifeq ($(TDCLI_SUPPORT),yes)
C_FLAG+= -DTDCLI_SUPPORT
endif
ifeq ($(ODBC_SUPPORT),yes)
C_FLAG+= -DODBC_SUPPORT
endif

ifeq ($(MYSQL_SUPPORT),yes)
C_FLAG+= -DMYSQL_SUPPORT
endif
ifeq ($(POSTGRESQL_SUPPORT),yes)
C_FLAG+= -DPOSTGRESQL_SUPPORT
endif

I_FLAG+= -I$(SRC_DIR)/include -I$(TOP_DIR)/bin/readline/include
L_FLAG+=

X_OBJ=
O_OBJ=$(patsubst %.c,%.o,$(C_SRC))
SUB_O_OBJ=$(patsubst %.c,%.o,$(SUB_C_SRC))
