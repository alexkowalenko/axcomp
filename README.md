# AX compiler

Compiler for the Oberon language, implemented in C++26, using the LLVM
compiler infrastructure for code generation.

Supports:

- The Oberon0 definition in Writh's _Compiler Construction_ (1996, 2017).
- Modules with the IMPORT statement.
- `REAL`s, `CHAR`s, `SET` types. (`SMALLINT`, etc. aliased to `INTEGER` (i64), `LONGREAL` aliased to `REAL` (double)).
- Nested `PROCEDURE`s (with closures in LLVM!).
- `RECORD` types, with base `RECORD` types.
- Multidimensional `ARRAY` types.
- Open `ARRAY`s.
- `POINTER TO` ...
- Oberon-2 statements like: `FOR`, `LOOP`, `EXIT`, `REPEAT`, `WHILE`, `CASE`, `RETURN`.

Beyond the Oberon-2 definition:

- UTF-8 source code, identifiers, and `CHAR`s.
- `STRING` type which supports UTF-8.

See the [AX Oberon-2/07 Language Report_](report/report.tex) in the [report](report) directory.

## Dependencies

To build the compiler:

- C++ 26 compiler - tested with Clang++ 21.1 on macOS.
- LLVM 21.0.0.
- UTFCPP v3.2.1 - support UTF-8 text (https://github.com/nemtrif/utfcpp.git).
- Boehm garbage collection
- ICU4C v78.1 – support UTF-8 text (https://github.com/unicode-org/icu.git).
- CMake 4.0 – build and test the compiler.

To test:

- GoogleTest release-1.17 - test suites (https://github.com/google/googletest.git).
- LLVM lit and FileCheck - regression tests.

To generate the language report – TeX.

## Usage

The compiler is `axcomp`, and the wrapper is `ax`. Use the `--help` option to list various compilation options.
See the [AGENTS.md](AGENTS.md) file for more information about running the compiler, and the layout of the source code
and test directories.

## Open Source Licensing information

The project is covered by the license in [LICENSE](LICENSE). The following are exempt and have the following licenses:

1. Some of the test code in [xtest](xtest), is originally from the oo2c project hosted at _Spirit of
   Oberon_ (https://github.com/Spirit-of-Oberon/oo2c.git). This is covered by the GNU General Public License v2.0.
2. Test code in [xtest/parse.ETHZ](xtest/parse.ETHZ) is from the ProjectOberon2013 hosted at _Spirit of
   Oberon_ (https://github.com/Spirit-of-Oberon/ProjectOberon.git). Copyright (C)2013 Niklaus Wirth (NW), Juerg
   Gutknecht (JG), Paul
   Reed (PR/PDR), as listed
   in [license.txt](https://github.com/Spirit-of-Oberon/ProjectOberon2013/blob/master/license.txt).