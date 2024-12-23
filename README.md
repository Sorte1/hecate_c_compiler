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