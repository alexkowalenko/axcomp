# AX compiler

Compiler for the Oberon-0 language and progressing towards implementing Oberon-2, implemented in C++17, using the LLVM compiler infrastructure for code generation.

Supports at the moment:

- The Oberon0 definition in Writh's _Compiler Construction_ (1996, 2017).
- Modules with the IMPORT statement.
- REALs, CHARs, SET types. (SMALLINT, etc. aliased to INTEGER (i64), LONGREAL aliased to REAL (double)).
- Nested PROCEDUREs (with closures in LLVM!).
- RECORD types, with base RECORD.
- Multidimensional ARRAY types.
- Open ARRAYs.
- POINTER TO ...
- Oberon-2 statements FOR, LOOP, EXIT, REPEAT, CASE, RETURN.

Beyond the Oberon-2 definition:

- UTF-8 source code, identifiers, and CHARs.
- STRING type which supports UTF-8.

See the [_AX Oberon-2/07 Language Report_](report/report.tex) in the [report](report) directory.

## Dependencies

To build the compiler:

- C++ 23 compiler - tested with Clang++ 17.0.
- LLVM 17.0.0.
- UTFCPP v3.2.1 - support UTF-8 text (https://github.com/nemtrif/utfcpp.git).
- Boehm garbage collection
- CMake - build and test the compiler.

To test:

- GoogleTest release-1.13.0 - test suites (https://github.com/google/googletest.git).
- Python 3.7 - to run the xtext test suite.

To generate the language report - TeX.

## Usage

How to run the compiler is explained in the [_AX Oberon-2/07 Language Report_](report/report.tex).

## Known issues

Limitations are listed in the [_AX Oberon-2/07 Language Report_](report/report.tex).

## Open Source Licensing information

The project is covered by the license in [LICENSE](LICENSE). The following are exempt and have the following licenses:

1. Some of the test code in [xtest](xtest), is originally from the oo2c project hosted at _Spirit of Oberon_ (https://github.com/Spirit-of-Oberon/oo2c.git). This is covered by the GNU General Public License v2.0.
2. Test code in [xtest/parse.ETHZ](xtest/parse.ETHZ) is from the ProjectOberon2013 hosted at _Spirit of Oberon_ (https://github.com/Spirit-of-Oberon/ProjectOberon.git). Copyright (C)2013 Niklaus Wirth (NW), Juerg Gutknecht (JG), Paul
Reed (PR/PDR), as listed in [license.txt](https://github.com/Spirit-of-Oberon/ProjectOberon2013/blob/master/license.txt).