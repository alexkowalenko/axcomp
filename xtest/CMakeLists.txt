#
# AX compiler
#
# Copyright © 2026 Alex Kowalenko
#

find_program(TEST_SUITE_LIT NAMES "lit" REQUIRED)

find_program(TEST_SUITE_FILECHECK NAMES "FileCheck" REQUIRED)


macro(xtest_add_lit_tests TEST_NAME TEST_DIR CFG_FILE)
    add_test(NAME ${TEST_NAME}
            COMMAND ${TEST_SUITE_LIT}
            --show-unsupported
            --show-xfail
            --max-time=300
            --timeout=10
            --config-prefix
            ${CFG_FILE}
            ${TEST_DIR}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endmacro()

configure_file(lit.parse.cfg.py.in ${CMAKE_CURRENT_SOURCE_DIR}/lit.parse.cfg.py @ONLY)
configure_file(lit.compile.cfg.py.in ${CMAKE_CURRENT_SOURCE_DIR}/lit.compile.cfg.py @ONLY)

# Parse tests
xtest_add_lit_tests(parse.oo2c parse.oo2c lit.parse)
xtest_add_lit_tests(parse.ETHZ parse.ETHZ lit.parse)

#xtest_add_test(demo)
#xtest_add_test(extra)

xtest_add_lit_tests(lang-const lang/const lit.compile)
xtest_add_lit_tests(lang-type lang/type lit.compile)
xtest_add_lit_tests(lang-var lang/var lit.compile)

## xtest_add_test(lang/record)
#xtest_add_test(lang/array)
## xtest_add_test(lang/openarray)
xtest_add_lit_tests(lang-set lang/set lit.compile)

xtest_add_lit_tests(lang-literals lang/literals lit.compile)
xtest_add_lit_tests(lang-expr lang/expr lit.compile)
xtest_add_lit_tests(lang-bool lang/bool lit.compile)
xtest_add_lit_tests(lang-char lang/char lit.compile)
## xtest_add_test(lang/string)
xtest_add_lit_tests(lang-numbers lang/numbers lit.compile)
xtest_add_lit_tests(lang-assignment lang/assignment lit.compile)
xtest_add_lit_tests(lang-conditional lang/conditional lit.compile)
## xtest_add_test(lang/pointers)

xtest_add_lit_tests(lang-procedure lang/procedure lit.compile)
xtest_add_lit_tests(lang-forward lang/forward lit.compile)
## xtest_add_test(lang/closures)
## xtest_add_test(lang/procedure-bound)

xtest_add_lit_tests(lang-if lang/if lit.compile)
xtest_add_lit_tests(lang-while lang/while lit.compile)
xtest_add_lit_tests(lang-repeat lang/repeat lit.compile)
xtest_add_lit_tests(lang-loop lang/loop lit.compile)
xtest_add_lit_tests(lang-for lang/for lit.compile)
#xtest_add_test(lang/case)
xtest_add_lit_tests(lang-return lang/return lit.compile)

#xtest_add_test(lang/import)

xtest_add_lit_tests(lib-assert lib/assert lit.compile)
#xtest_add_test(lib/inc)
#xtest_add_test(lib/char)
xtest_add_lit_tests(lib-halt lib/halt lit.compile)
#xtest_add_test(lib/maths)
xtest_add_lit_tests(lib-set lib/set lit.compile)
#xtest_add_test(lib/size)
xtest_add_lit_tests(lib-write lib/write lit.compile)
