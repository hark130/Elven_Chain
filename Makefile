CC      = gcc
CFLAGS  = -g
OUT		= Elf_Scout
RM      = rm -f

all: 
	$(CC) $(CFLAGS) -o $(OUT) Elf_Details.c Elven_Chain.c Harklehash.c

clean:
	$(RM) *.o *.i