
C_FLAG+=-D_DEBUG_ON_
I_FLAG+= -I$(TOP_DIR)/src
L_FLAG+= -lcunit

#---------------------------special rules----------------------------
#
.phony: all test
all: $(E_OBJ)

$(E_OBJ): $(O_OBJ)
	@echo "[LD] $(E_OBJ)"
	@$(LD) $(L_FLAG) -o $@ $^

test:
	@echo "### running $(E_OBJ)"
	@export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:$(TOP_DIR)/bin/cunit/lib && ./$(E_OBJ)
