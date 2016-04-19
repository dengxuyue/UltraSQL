
PQ_HOME=$(HOME)/local/pgsql/9.5
MY_HOME=$(HOME)/local/mysql-cluster/7

export PQ_HOME MY_HOME TDODBC

ODBC_SUPPORT=no
# default: not supported
TDCLI_SUPPORT=no
ifeq ($(tdcli),yes)
TDCLI_SUPPORT=yes
endif
export ODBC_SUPPORT TDCLI_SUPPORT

# default: supported
MYSQL_SUPPORT=yes
ifeq ($(mysql),no)
MYSQL_SUPPORT=no
endif
# default: supported
POSTGRESQL_SUPPORT=yes
ifeq ($(postgresql),no)
POSTGRESQL_SUPPORT=no
endif
export MYSQL_SUPPORT POSTGRESQL_SUPPORT

TOP_DIR=..
SRC_DIR=.

C_SRC=main.c
SUB_C_SRC=interface/dbi.c \
          parser/dots.c parser/coarse.c parser/miniparser.c parser/gram.c parser/ptree.c \
          response/parcels.c response/pager.c response/myresult.c response/export.c \
          util/debug.c util/cmdhistory.c util/passwd.c util/profile.c util/setting.c util/helper.c 

SUB_DIR=parser interface response util
E_OBJ=usql

include $(SRC_DIR)/build/config.mk

L_FLAG+=-lncurses -lreadline -lhistory

ifeq ($(MYSQL_SUPPORT),yes)
SUB_C_SRC+= interface/myclient.c response/myresult.c 
L_FLAG+= -L$(MY_HOME)/lib -lmysqlclient
endif
ifeq ($(POSTGRESQL_SUPPORT),yes)
SUB_C_SRC+= interface/pqlib.c
L_FLAG+= -L$(PQ_HOME)/lib -lpq
endif

ifeq ($(TDCLI_SUPPORT),yes)
SUB_C_SRC+= interface/tdcli.c
L_FLAG+= -lcliv2
endif
ifeq ($(ODBC_SUPPORT),yes)
SUB_C_SRC+= interface/odbc.c
L_FLAG+= -lodbc
endif

.phony: all sub_all clean sub_clean

all: env_macro sub_all $(E_OBJ)
sub_all: $(addsuffix _all,$(SUB_DIR))

clean: sub_clean
sub_clean: $(addsuffix _clean,$(SUB_DIR))

env_macro: Makefile
	@echo "export LD_LIBRARY_PATH=\044LD_LIBRARY_PATH:$(PQ_HOME)/lib:$(MY_HOME)/lib" > env_macro

$(E_OBJ): $(O_OBJ) $(SUB_O_OBJ)
	@echo "[LD] $@"
	@$(LD) -o $@ $^ $(L_FLAG)

include $(SRC_DIR)/build/rule.mk

define build_sub_dir
$(1)_$(2):
	@make -C $1 $2

endef

$(foreach dir,$(SUB_DIR),$(eval $(call build_sub_dir,$(dir),all)))
$(foreach dir,$(SUB_DIR),$(eval $(call build_sub_dir,$(dir),clean)))
