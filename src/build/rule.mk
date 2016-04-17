
%.o: %.c
	@echo "[CC] $<"
	@$(CC) -c $(C_FLAG) $(I_FLAG) -o $@ $<

clean:
	@rm -f $(O_OBJ) $(E_OBJ) $(X_OBJ)
