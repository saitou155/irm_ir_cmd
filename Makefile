CC      = cc
OBJS    = irm_ir_cmd.o

irm_ir_cmd: $(OBJS)
	$(CC) -Wall -g -o $@ $(OBJS) -lusb-1.0 -ljson-c

.c.o:
	$(CC) -c -g $<

clean:
	@rm -f $(OBJS)
	@rm -f irm_ir_cmd
