
PQ_HOME=$(HOME)/local/pgsql/9.5/
export PQ_HOME

TOP_DIR=../..
#SUB_TEST=parser_miniparser interface_tdcli interface_pqlib interface_odbc util_cmdhistory util_passwd
SUB_TEST=parser_miniparser interface_pqlib interface_odbc util_cmdhistory util_passwd

parser_miniparser_dir=parser/miniparser
interface_tdcli_dir  =interface/tdcli
interface_pqlib_dir  =interface/pqlib
interface_odbc_dir   =interface/odbc
util_cmdhistory_dir  =util/cmdhistory
util_passwd_dir      =util/passwd

all: sub_all
sub_all: $(addsuffix _all,$(SUB_TEST))

test: sub_test
sub_test: $(addsuffix _test,$(SUB_TEST))

clean: sub_clean
sub_clean: $(addsuffix _clean,$(SUB_TEST))

define build_sub_dir
$(1)_$(2):
	@make -C $($(1)_dir) $2

endef

$(foreach dir,$(SUB_TEST),$(eval $(call build_sub_dir,$(dir),all)))
$(foreach dir,$(SUB_TEST),$(eval $(call build_sub_dir,$(dir),test)))
$(foreach dir,$(SUB_TEST),$(eval $(call build_sub_dir,$(dir),clean)))
