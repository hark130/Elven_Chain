# Elven_Chain
This extremely light repository is made of very fine ELF links. This project is treated, in all ways, like light code, including when determining programming proficiency. The project has a spell.h ERRNO chance of 20%, a maximum key strokes per character (KSPC) bonus of +4, and an Microsoft check penalty of –2.

## VERSION
1. v0.1.0 - Implements ELF Header
2. prgrm_hdr - Current working branch, implementing the Program Header (future v0.2.0)

## TO DO
    [ ] Rename Harklehash to Harkledict
    [X] Implement ELF Header
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
        [X] Entry Point
        [X] Program Header Table Offset
        [X] Section Header Table Pointer
        [X] Flags
        [X] ELF Header Size
        [X] Program Header Table Size
        [X] Program Header Table Number of Entries
        [X] Section Header Table Size
        [X] Section Header Table Number of Entries
        [X] Index to Section Header Table with Section Names
    [/] Implement Program Header
        [X] Segment Type
        [X] 64-bit Flags
        [X] Offset of the Segment
        [X] Virtual Address of the Segment
        [X] Segment's Physical Address
        [X] Size of the Segment in File Image (bytes)
        [X] Size of the Segment in Memory (bytes)
        [X] 32-bit Flags
        [/] Alignment
    [ ] Implement Section Header
    [ ] Implement Program Data
    [ ] Implement Section Data
    [ ] Implement ELF Integrity Validator
        [ ] ELF Header actually adds up to ELF Header Size
        [ ] Index to Section Header Table with Section Names <= Section Header Table Number of Entries
        [ ] Section Header type SHT_GROUP should only appear in relocatable objects (e_type == ET_REL)
        [ ] Alignment should be 0, 1, or a positive integral power of 2, with p_vaddr equating p_offset modulus p_align
    [ ] Test sectionsToPrint
    [ ] If a previous function discovers an "ORC_FILE", ensure print_program_header() isn't called
    [ ] Better way to convert a char value to int?
    [X] What about multi-byte char values?
    [ ] What happens if I hide something in the Elf Header PAD (offset 0x09)?
    [X] Add Elf Header PAD to Elf_Detail struct (see: NOTES TO THE WORLD)
    [ ] Move init_*() function calls in parse_elf() to just-in-time code block calls
    [ ] Implement [ISA macros](http://www.sco.com/developers/gabi/latest/ch4.eheader.html) for 83 - 243
    [ ] What happens if I modify the entry point of an ELF File?  1337 h4x?
    [ ] Look for memory leaks (valgrind)
    [ ] Dump out of elf_parser() at offset 0x10 if Endianness doesn't match existing macro
    [ ] Match allocation error handling from read_program_header() into read_elf() [see: "ERROR: Allocation..."]
    [ ] Rename Program Header 64-bit flag macros to something agnostic of processor "class"
    [ ] Refactor Program Header segment parsing to memcpy struct into Program Header Details array element (will it work?)

## NOTES TO THE WORLD (and future Hark)
### Implementing new members of the "Header"_Details struct involves updating the following:
* parse_"header"()
* print_"header"_details()
* kill_"header"()
### Compilation
```
    clear
    gcc -c Elf_Details.c
    gcc -c Elven_Chain.c
    gcc -c Harklehash.c
    gcc -o Elf_Scout.exe Elf_Details.o Elven_Chain.o Harklehash.o
    ./Elf_Scout.exe Elf_Scout.exe

```
-or-
```
    clear; gcc -o Elf_Scout.exe Elf_Details.c Elven_Chain.c Harklehash.c; ./Elf_Scout.exe Elf_Scout.exe

```
-or-
```
    clear; make; ./Elf_Scout.exe Elf_Scout.exe

```
### Great Sources
* https://github.com/michalmalik/linux-re-101 (ELF RE collection)
* http://imgur.com/a/JEObT (Graphical ELF representation)
* https://www.youtube.com/watch?v=O_Y-eQJGg9g (Open SecurityTraining: Life of Binaries)
    * ePnt32/64 - start of executable code
    * prgrmHdrEntrNum - number of "segments"
    * sectHdrSectNms - index into the section header that contains string table
    * NOTE: For a program header segment, only "type" PT_LOAD gets loaded into memory
    * If PH LOAD segment filesz == vmemsize, there's no BSS
