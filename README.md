# Hecate C Compiler README

This document explains how to use the Hecate C Compiler to generate Hecate Assembly (HASM) files. The compiler translates LLVM IR (`.ll` files) into assembly that can be run on the Hecate Virtual Machine.

---

## Overview

The Hecate C Compiler takes LLVM IR files and converts them into Hecate Assembly. LLVM IR is an intermediate representation that you can generate from C code using Clang.

The project is a work in progress and aims to eventually support the full specification of Hecate Assembly (HASM). Some instructions and features of HASM are not yet implemented in the assembler, and limitations may exist in the current compiler.

---

## Getting Started

### Building the Compiler

To build the compiler, you need CMake and a C++ compiler. Follow these steps:

1. Create a `build` directory:
```bash
mkdir build
cd build
```
2. Run CMake and compile
```bash 
cmake ..
make
```
   
## Generating LLVM IR

To compile a C file into LLVM IR, use Clang with the following options:
```bash
clang -S -emit-llvm <path-to-your-file.c>
```
For example, to compile a file named `test.c`:
```bash
clang -S -emit-llvm test.c
```
This will produce a file called `test.ll`.


## Running the Compiler

Use the generated LLVM IR file as input to the Hecate C Compiler:
```bash
./hecate_c_compiler_c <input.ll> -o <output.hasm>
```
For example, if your LLVM IR file is `test.ll`, you can generate `test.hasm` with:

## Features

The Hecate C Compiler:

* Translates LLVM IR into Hecate Assembly (`.hasm`) compatible with the Hecate Virtual Machine.
* Converts basic control flow, memory operations, and arithmetic into HASM.
* Handles global strings by initializing them in memory during assembly generation.
* Provides experimental support for translating function calls and specific HASM instructions.
* Is a work in progress, with plans to support the full specification of Hecate Assembly in the future.

## Notes
* Use Clang to generate the `.ll` files needed by the compiler.
* The compiler assumes the target architecture supports the required LLVM features.
* The assembler and virtual machine may not yet support all instructions and features in the HASM specification.


## Example output

```c++
void inspect(void *p1);
int main() {
    char const *str = "Hello, world";
  for (int i = 1; i <= 6969; i++) {

     inspect(&i);
  }


  return 0;
}
```

```asm
; --- Global Data Section ---
;.str:
;  db  72,101,108,108,111,44,32,119,111,114,108,100,0 ; "Hello, world"
; --- End of Global Data ---

main:
Block0:
  ; local space: @1000
  ; local space: @1004
  ; local space: @1008
  store @1000, R0
  store @1004, R2
  store @1008, R1
  jmp @Block1
Block1:
  load R3, @1008
  ; icmp sle R3, 6969
  cmp R3, 6969
  cmp R4, R0
  je @Block4
  jmp @Block2
Block2:
  inspect @1008 ; debug
  jmp @Block3
Block3:
  load R5, @1008
  ; plus R6 = R5 + R1
  loadReg R6, R5
  add R6, R1
  store @1008, R6
  jmp @Block1
Block4:
  ; returning R0
  ret
  halt ; main ended

```

