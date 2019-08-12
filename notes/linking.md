# Chapter 7 Linking

## Overview

```
      +------------+      
      |*.c         |      
      |Source Files|      
      +------------+      
             |            
        Translators       
             |            
             |            
             v            
+------------------------+
|*.o                     |
|Relocatable Object Files|
+------------------------+
             |            
          Linker          
             |            
             |            
             v            
 +----------------------+ 
 |Executable Object File| 
 +----------------------+ 
```

### Object Files

Object files come in three forms:

 - Relocatable object file

 - Executable object file

 - Shared object file

   can be linked dynamically at either load time or run time.

Structure of ELF object file:

```
+-------------+
|  ELF header |
+-------------+
|  Sections   |
+-------------+
|  Section    |
| header table|
+-------------+
```

Object files contains several sections:

 - *text*    machine code 
 - *rotext*  read-only data
 - *data*    initialized global and static C variables 
 - *bss*     uninitialized global and static C variables, and global or static variables that are initialized to zero
 - *symtab*  a table with information about functions and global variables
 -  ...

### Symbol Table

**Symbol**

```c
typedef struct {
    int   name;      // String table offset, name of the variable
    char  type:4,    // Function or data
          binding:4; // Global or local
    char  reserverd; 
    short section;   // Section header index
                     // There are there speical pseudosections that don't have entries in the section header
                     // - COMMON  symbols for unintialized global variables 
                     // - UNDEF   symbols that are referenced but not defined
                     // - ABS     symbols that should not be relocated
    long  value;     // Section offset or absolute address
    long  size;      // Object size in bytes
} Elf64_Symbol;
```

For a module *m*, there are three different kinds of symbols:

- Global symbols that are defined by module *m* and that can be referenced by other modules.
- External symbols that are referenced by module *m* and but defined by some other module.
- Local symbols.

## Static Linking

**Steps:**

1. Symbol resolution

   Associate each symbol *reference* with exactly one symbol *definition*.

2. Relocation

   Associate a memory location with each symbol definition.

### Symbol Resolution

For local symbol, the compiler allows only one define for each local symbol per module.
And for reference to global symbol, the compiler will generate a linker symbol entry, and leave it for linker to handle.

#### Duplicate Symbol Names

The compiler classify global symbols as either strong or weak.

Declarations of functions or variables get weak symbols. Definitions gets strong symbols.
It's easy to say whether a function is strong a weak. However, for variables, it is ambiguous.
Uninitialized variables get weak symbols(COMMON section), and initialized variables get strong symbols.
If a function is weak, the section of its symbol is *UNDEF*. If a variable is weak, the section of its symbol is *COMMON*.

Rules for dealing duplicate symbols:

- Multiple strong symbols with the same name are not allowed.
- Given a strong symbols and multiple weak symbols with the same name, choose the strong symbol.
- Given multiple weak symbols with the same name, for variables choose any of the weak symbols, and for functions report an error.

Multiple weak symbols of variables can cause terrible bugs. To avoid bugs caused by global variables:

 - use static if you can
 - initialize if you define a global variable
 - use *extern* if you reference an external global variables

#### Relocation

 **Steps:**:

- Merge separate sections into single section.
- Relocate symbols from relative positions to final absolute positions.
- Update all reference to those symbols to reflect their new positions.

##### How linkers handle duplicate symbol name of local variables in multiple object file?

用中文记录一下这个细节。

我最开始的疑惑是：假设有两个object它们有相同名称的本地变量（`static`），并且两个变量处于相同的section，比如下面的*t.c*和*s.c*。在relocation中，它们的不同的sections先合并，两个相同名称的symbol也就该被合并到相同的section。那么在上述的步骤三中，把reference的地址改为合并后symbol的绝对地址时该怎么处理名字相同的冲突。这是因为我对relocation的细节还不够清晰，以为步骤三还必须依靠symbol的信息。其实，目标文件合并好之后，更新reference的时候不一定不依赖symbol table，有的情况根据relocation entries提供的信息就足够了。

*t.c*
```c
// t.c
static int local1 = 2;
void fn1() {
    local1++;
}
```

*s.c*

```c
// s.c
static int local1 = 2;
static int local2 = 2;
static int local3 = 2;
void fn2() {
    local1++;
    local2++;
    local3++;
}
```

要具体了解这个过程需要先知道object文件中的 relocation 信息的储存形式。下面是ELF relocation entry的定义：

```c
typedef struct {
    long offset;    // Section offset of the reference to relocate
    long type:  32, // Relocation type
		 symbol:32; // Symbol table index
    long addend;    // Constant part of relocation expression
} Elf64_Rela;
```

值得注意的是这里的`symbol` 只记录了symbol table index，没有关于symbol的具体信息。那么应该怎么处理reference的地址更新呢？答案就在有点奇怪的`addend`中。

*objdump -r* & *objdump -d*

```
s.o:     file format elf64-x86-64

RELOCATION RECORDS FOR [.text]:
OFFSET           TYPE              VALUE 
0000000000000002 R_X86_64_PC32     .data+0x0000000000000003
0000000000000009 R_X86_64_PC32     .data-0x0000000000000001
0000000000000010 R_X86_64_PC32     .data-0x0000000000000005


Disassembly of section .text:

0000000000000000 <fn2>:
   0:	83 05 00 00 00 00 01 	addl   $0x1,0x0(%rip)        # 7 <fn2+0x7>
   7:	83 05 00 00 00 00 01 	addl   $0x1,0x0(%rip)        # e <fn2+0xe>
   e:	83 05 00 00 00 00 01 	addl   $0x1,0x0(%rip)        # 15 <fn2+0x15>
  15:	c3   
```

需要解释一下TYPE列中 `R_X86_64_PC32` 是相对PC的内存地址的意思，访存的时候要加上PC寄存器的值。

可以看到在Relocation的信息中 VALUE的记录是`.data+addend`， `.data`应该是`symbol`记录的内容，记录的内容是`symbol`所属的section。在Relocation中，`R_X86_64_PC32`这种类型计算地址的方式是：

`ADDR(r.symbol)  + r.addend - (ADDR(section) + r.offset)`.

这里`ADDR(r.symbol)`是reference的symbol所属的section的run-time地址，`(ADDR(section) + r.offset)`是reference 的run-time地址。`addend`是指向symbol本身的偏移加上PC相对reference的偏移，即`addend=ref_sub_pc+symbol_offset`。

比如上面的三个个reference，它的地址计算结果分别为：

```
ADDR(.data) + 0x3 - (0x2 + ADDR(.text))
ADDR(.data) - 0x1 - (0x9 + ADDR(.text))
ADDR(.data) - 0x5 - (0x10 + ADDR(.text))
```

在访问它们的内存时加上PC的值，访问的地址就是：

```
ADDR(.data)+0x3-(0x2 +ADDR(.text))+(ADDR(.text)+0x7) = ADDR(.data)+0x8     
ADDR(.data)-0x1-(0x9 +ADDR(.text))+(ADDR(.text)+0xe) = ADDR(.data)+0x4
ADDR(.data)-0x5-(0x10+ADDR(.text))+(ADDR(.text)+0x15)= ADDR(.data)+0x0
```

从上面的地址计算方式可以看出，在relocation过程中reference地址的更新不会根据symbol的名称决定，直接通过symbol相对section的偏移就能够区分不同的section了。

##### Relocation algorithm

```
for s in section {
    for r in s's relocation_entries {
    	refptr = s + r.offset // position to update the reference in file
    	if r.type == R_X86_64_PC32 {
    		refaddr = ADDR(s) + r.offset // runtime address
    		*refptr = ADDR(r.symbol) + r.addend - refaddr
    	}
    	if r.type == R_X86_64_32 {
    		*refptr = ADDR(r.symbol) + r.addend
    	}
    }
}
```

#### Static Library

Static libraries are just **archives** of object modules(.o files). Related functions can be compiled into separate object modules and then packaged in a single library file. At link time, the linker will only copy the object modules that are referenced by the program.

During the symbol resolution phase, the linker scans archives and relocatable object files from left to right in the same order that they appear on the command line.

*Resolution algorithm of static library*

```
E = {} // a set of relocatable object files 
	   // that will be merged to form the executable
U = {} // a set of unresolved symbols(referenced but not defined)
D = {} // a set of symbols that have been defined by mudules in E
for f in input_files {
	if f is an object file {
		add f E
		update U
		update D
	}
	if f is an archive file {
		do {
			for obj in f {
				if obj contains defined symbols that appeared in U {
					add obj E
					update U
					update D
				}
			}
		} while U and D no longer change
	}
}
```

From the strategy of resolution of static library, we can see that the order of input on command line is significant.

**Features of static library:**

- Archiver allows incremental updates: recompile function that changes and replace .o in archive.
- Have to recompile every executable if library updates.
- For some frequently used library it's a waste of scarce memory system resources, because the code for those libs may be duplicated in the text segment of each running process.

### Dynamic Linking with Shared Library

A shared library is an object module that, at either run time or load time, can be loaded at an arbitrary memory address and linked with a program in memory.

"Shared" in two different ways:

- The code and data in a shared library is shared by all executables that reference it, but static library is embedded in the executables.
- A single copy of .text section of a shared library can be shared by different running processes.

### gcc command

create static lib

```bash
gcc -c *.c
ar rcs libname.a *.o
```

link with static lib

```bash
gcc -static -o main main.c ./libname.a
# or
gcc -static -o main main.c -L. -lname
# -L. tells the linker to look for libname.a in the current directory
```

create shared lib

```bash
gcc -shared -fpic -o libname.so *.c
# -fpic means position-independent code
```

create executable that can be linked with shared libs  at run time

```bash
gcc -o main main.c ./libname.so
```

create dynamically linked executable

```bash
gcc -rdynamic -o main main.c -ldl
```

### PIC(Position-Independent Code)

// TODO