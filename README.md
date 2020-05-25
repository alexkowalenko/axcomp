# axcomp

AX compiler

Compiler for the Oberon-0 language and progressing towards implementing Oberon-2, implemented in C++17, using the LLVM Compiler infrastructure for code generation.
Using version 10.0.0 of the LLVM toolkit.

Supports at the moment:

- The Oberon0 definition in Writh's _Compiler Construction_ (2017).

- Modules with the IMPORT statement
- REALs, CHARs, types.
- Open ARRAYs.
- POINTER TO ...
- Oberon-2 statements.

Beyond the definitions:

- UTF-8 source code, identifiers, and CHARs.
- STRING type which supports UTF-8.

See the _AX Oberon-0 Language Report_ in the `report` directory.
