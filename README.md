# Elven_Chain
This extremely light repository is made of very fine ELF links. This project is treated, in all ways, like light code, including when determining programming proficiency. The project has a spell.h ERRNO chance of 20%, a maximum key strokes per character (KSPC) bonus of +4, and an Microsoft check penalty of –2.

## TO DO
    [ ] Rename Harklehash to Harkledict
    [/] Implement ELF Header
        [X] Magic Number
        [X] Class
        [X] Endianness
        [X] ELF Version
        [X] Target OS ABI
        [X] ABI Version
        [X] Pad
        [X] ELF Type
        [X] ISA
        [X] Object File Version
        [ ] Entry Point
        [ ] Program Header Table Pointer
        [ ] Section Header Table Pointer
        [ ] Flags
        [ ] ELF Header Size
        [ ] Program Header Table Size
        [ ] Program Header Table Number of Entries
        [ ] Section Header Table Size
        [ ] Section Header Table Number of Entries
        [ ] Index to Section Header Table with Section Names
    [ ] Implement Program Header
        [ ] Segment Type
        [ ] 64-bit Flags
        [ ] Offset of the Segment
        [ ] Virtual Address of the Segment
        [ ] Segment's Physical Address
        [ ] Size of the Segment in File Image (bytes)
        [ ] Size of the Segment in Memory (bytes)
        [ ] 32-bit Flags
        [ ] Alignment
    [ ] Implement Section Header
    [ ] Implement Program Data
    [ ] Implement Section Data
    [ ] Better way to convert a char value to int?
    [X] What about multi-byte char values?
    [ ] What happens if I hide something in the Elf Header PAD (offset 0x09)?
    [X] Add Elf Header PAD to Elf_Detail struct (see: NOTES TO THE WORLD)
    [ ] Move init_*() function calls in parse_elf() to just-in-time code block calls
    [ ] Implement [ISA macros](http://www.sco.com/developers/gabi/latest/ch4.eheader.html) for 83 - 243
    [ ] What happens if I modify the entry point of an ELF File?  1337 h4x?

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
