# Repository Guidelines

## Project Structure & Module Organization

* Core compiler sources live inside `src/compiler/`, grouped by lexer/parser, AST + visitors, importer logic, and
  LLVM-based code generation.
* `src/cmd/` provides the `axcomp` CLI,
* while `src/runtime/` holds the Oberon modules and C++ glue that become the runtime library `libAx`.
* Tests split between GoogleTest suites in `tests/` and language regression suites under `xtest/` driven by LLVM's
  `lit`.
* Keep helper CMake modules in `cmake/`; generated directories like `build/` should stay disposable.

## Build, Test, and Development Commands

* Use out-of-source builds; the loop below covers most workflows:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_TESTING=ON
cmake --build build --target axcomp install
ctest --test-dir build --output-on-failure
```

The compiler is installed `bin` in the main source directory, when the install command is omitted.

* Use `ctest -R lang` for compiler regressions.
* Use `ctest -R lib` for runtime library regressions.
* Use `ctest -R parse` for parser regressions.

* Docs build with `cmake --build build --target docs`
* Fuzzers come from configuring with `-DENABLE_FUZZTEST=ON`.

## Running the ax Compiler

To run the ax compiler:

```shell
bin/ax -m prog.mod
```

To build and run a simple program:

```shell
bin/ax -m --run prog.mod
```

## Coding Style & Naming Conventions

* `.clang-format` (LLVM base, four spaces, 99-column limit) and `.clang-tidy` guard the C++ style; run
  `clang-format -i <file>.{cc,hh}` before committing.
* Stick with `.cc/.hh` pairs, PascalCase for types, lower_snake_case for helper functions, and SCREAMING_SNAKE for
  constants.
* Reuse the smart-pointer helpers in `ast.hh` and keep new code inside existing namespaces unless a fresh module
  boundary is required.
* C++ 23 or later can be used.

## Testing Guidelines

* Add GoogleTest cases next to the subsystem you extend and register them in `tests/CMakeLists.txt`.
* Follow the `component.N.cc` naming to keep diffs narrow and fixtures readable.
* `xtest` use LLVM lit and FileCheck to verify compiler output and runtime behaviour.
  At the end of `*.Mod` and `*.mod` files there is a comment with RUN: and CHECK: lines which specify how to run the
  compiler and check the output.
* Fuzzing stays opt-in—tune runtime with `-DFUZZ_RUNTIME=<seconds>` and store corpora under `fuzz_test/corpus*`.

## Commit & Pull Request Guidelines

* Recent history favours short, imperative subjects (`Get rid of deprecated std::codecvt_utf8`,
  `Documentation changes`).
* Keep the summary ≤80 characters, expand on motivation plus test evidence in the body, and link issues or report
  sections that justify the change.

## Environment & Configuration Tips

* Boehm GC, UTFCPP, argparse, and GoogleTest are fetched via CPM during configure, so ensure those toolchains exist
  locally.
* Sanitiser, analyser, fuzzer, and PCH toggles live near the top of `CMakeLists.txt`.
* Tools like `fd` and `rg` are installed.
