# Elven_Chain
This extremely light repository is made of very fine ELF links. This project is treated, in all ways, like light code, including when determining programming proficiency. The project has a spell.h ERRNO chance of 20%, a maximum key strokes per character (KSPC) bonus of +4, and an Microsoft check penalty of –2.

## TO DO
    [ ] Rename Harklehash to Harkledict
    [ ] Implement Program Header
    [ ] Implement Section Header
    [ ] Implement Program Data
    [ ] Implement Section Data
    [ ] Better way to convert a char value to int?
    [ ] What about multi-byte char values?
    [ ] What happens if I hide something in the Elf Header PAD (offset 0x09)?
    [ ] Add Elf Header PAD to Elf_Detail struct (see: NOTES TO THE WORLD)

## NOTES TO THE WORLD (and future Hark)
### Implementing new members of the Elf_Details struct involves updating the following:
* parse_elf()
* print_elf_details()
* kill_elf()
### Compilation
```
    clear
    gcc -c Elf_Details.c
    gcc -c Elven_Chain.c
    gcc -c Harklehash.c
    gcc -o Elf_Scout Elf_Details.o Elven_Chain.o Harklehash.o
    ./Elf_Scout Elf_Scout

```
-or-
```
    clear; gcc -o Elf_Scout Elf_Details.c Elven_Chain.c Harklehash.c; ./Elf_Scout Elf_Scout

```
-or-
```
    clear; make; ./Elf_Scout Elf_Scout

```