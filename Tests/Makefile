CC      = gcc
CFLAGS  = -g
RM      = rm -f

all: 
	$(CC) $(CFLAGS) -o TEST_ccti.exe TEST_convert_char_to_int.c ../Elf_Details.c ../Harklehash.c
	$(CC) $(CFLAGS) -o TEST_cctu64.exe TEST_convert_char_to_uint64.c ../Elf_Details.c ../Harklehash.c
	$(CC) $(CFLAGS) -o TEST_cu64tu32.exe TEST_convert_uint64_to_uint32.c ../Elf_Details.c ../Harklehash.c

clean:
	$(RM) *.o *.i *.exe