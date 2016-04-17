TOP_DIR=../..
SRC_DIR=..

C_SRC=dots.c coarse.c miniparser.c gram.c ptree.c

include $(SRC_DIR)/build/config.mk
X_OBJ+=scan.h gram.h scan.c gram.c

all: $(O_OBJ)

scan.c: scan.l
	@echo [LEX $<]
	@$(LEX) $< > $@
gram.c: gram.y
	@echo [YACC $<]
	@$(YACC) -d -o $@ $<

miniparser.c: gram.c 
gram.c: scan.c

include $(SRC_DIR)/build/rule.mk
