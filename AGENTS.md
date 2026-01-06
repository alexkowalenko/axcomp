# Repository Guidelines

## Project Structure & Module Organization
* Core compiler sources live inside `src/compiler/`, grouped by lexer/parser, AST + visitors, importer logic, and LLVM-based code generation. `src/cmd/` provides the `axcomp` CLI, while `src/runtime/` holds the Oberon modules and C++ glue that become `libAx`. 
* Tests split between GoogleTest suites in `tests/` and language regression suites under `xtest/` driven by `tester.py`. 
* Keep helper CMake modules in `cmake/`; generated directories like `build/` should stay disposable.

## Build, Test, and Development Commands
Use out-of-source builds; the loop below covers most workflows:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_TESTING=ON
cmake --build build --target axcomp install
ctest --test-dir build --output-on-failure
```
Use `ctest -R xtest` for parser regressions. 
Docs build with `cmake --build build --target docs`; fuzzers come from configuring with `-DENABLE_FUZZTEST=ON`.

## Coding Style & Naming Conventions
* `.clang-format` (LLVM base, four spaces, 99-column limit) and `.clang-tidy` guard the C++ style; run `clang-format -i <file>.{cc,hh}` before committing. 
* Stick with `.cc/.hh` pairs, PascalCase for types, lower_snake_case for helper functions, and SCREAMING_SNAKE for constants. 
* Reuse the smart-pointer helpers in `ast.hh` and keep new code inside existing namespaces unless a fresh module boundary is required.

## Testing Guidelines
* Add GoogleTest cases next to the subsystem you extend and register them in `tests/CMakeLists.txt`. 
* Follow the `component.N.cc` naming to keep diffs narrow and fixtures readable. `xtest` scenarios require updating both the Oberon source and its `.res`/`.exp` expectation files; commit those alongside the change and rerun `ctest -R xtest`. 
* Fuzzing stays opt-in—tune runtime with `-DFUZZ_RUNTIME=<seconds>` and store corpora under `fuzz_test/corpus*`.

## Commit & Pull Request Guidelines
* Recent history favours short, imperative subjects (`Get rid of deprecated std::codecvt_utf8`, `Documentation changes`). 
* Keep the summary ≤72 characters, expand on motivation plus test evidence in the body, and link issues or report sections that justify the change. 
* Each PR should describe the user-visible effect, mention doc/report updates, paste the `ctest` (and `xtest/tester.py` when relevant) commands you ran, and flag ABI or language-surface changes so reviewers can coordinate runtime bumps.

## Environment & Configuration Tips
* Boehm GC, UTFCPP, and GoogleTest are fetched via CPM during configure, so ensure those toolchains exist locally. 
* Sanitizer, analyzer, fuzzer, and PCH toggles live near the top of `CMakeLists.txt`.
