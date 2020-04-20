#!env python3
#
# AX compiler
#
# Copyright © 2020 Alex Kowalenko
#

from glob import glob
from pathlib import Path
import io
import os
import configparser
from colored import fg, bg, attr

import argparse

install_dir = "../../bin"
lib_dir = os.environ['AXLIB_PATH']
axlib_dir = lib_dir + ":."

# Config paramaters
test_cfg = "test.ini"

pre_test = ""
post_test = ""
link_objs = ""
exclude = ""

compiler = f"{install_dir}/ax"
linker = f"clang++ ../main.cc -L {lib_dir} -lAx "

red = fg('red_1')
restore = attr('reset')


def do_clang(stem):
    obj = stem + ".o"
    cmd = linker + f"{link_objs} {obj}"
    # print(cmd)
    ret = os.system(cmd)
    if (ret != 0):
        print(red + "link " + restore, end="")
        os.system(f"rm -f a.out {obj} {stem}.def result.diff.txt result.txt")

    os.system("./a.out > result.txt")
    res = stem + ".res"
    ret = os.system(
        f"diff --strip-trailing-cr {res} result.txt > result.diff.txt")
    if(ret != 0):
        print(red + "run " + restore, end="")
        fail = stem + ".fail"
        os.system(f"mv result.txt {fail}")
    os.system(f"rm -f a.out {obj} {stem}.def result.diff.txt result.txt")

    return (ret == 0)


def do_test(t):
    stem = Path(t).stem
    fail = stem + ".fail"

    # Compile file
    cmd = f"{compiler} -L {axlib_dir} --output_funct {t} > result.txt"
    # print(cmd)
    ret = os.system(cmd)
    if ret:
        os.system(f"mv result.txt {fail}")
        print(red + "compile " + restore, end="")
        return 0

    # Check llvm output
    exp = stem + ".exp"
    asm = stem + ".ll"
    cmd = f"diff --strip-trailing-cr {exp} {asm} > result.diff.txt"
    # print(cmd)
    ret = os.system(cmd)
    if(ret != 0):
        os.system(f"mv {asm} {fail}")
        os.system(f"rm -f {stem}.o {stem}.def")
    else:
        os.system(f"rm -f {fail} {asm}")
    os.system("rm -f result.diff.txt")
    if (ret != 0):
        print(red + "llir " + restore, end="")
        return 0
    # compile
    return do_clang(stem)


# Perform tests on a list of filename
def do_tests(l):
    global exclude
    global pre_test

    # Pre_test
    if(len(pre_test) != 0):
        print("  pre_test")
        compiled_fstring = compile(
            pre_test, '<fstring_from_file>', 'eval')
        formatted_output = eval(compiled_fstring)
        # print(formatted_output)
        os.system(formatted_output)

    count = 0
    fails = 0
    for x in l:
        if x == exclude:
            continue
        print(f"  : {x} ", end="")
        result = do_test(x)
        count += 1
        if(not result):
            print(red + f"-> :Fail" + restore, end="")
            fails += 1
        print()

    if(len(post_test) != 0):
        print("  post_test")
        os.system(post_test)
    print(f"Ran {count} tests with {fails} fails.")

    return fails


# Perform tests on a list of directories
def do_tests_dir(d):
    global pre_test
    global post_test
    global link_objs
    global exclude

    print(f"Tests: {d}")
    os.chdir(d)

    pre_test = ""
    post_test = ""
    link_objs = ""
    # Read config file if exists
    if os.path.isfile(test_cfg):
        config = configparser.ConfigParser()
        config.read(test_cfg)
        pre_test = config.get("compile", "pre_test")
        post_test = config.get("compile", "post_test")
        link_objs = config.get("compile", "link_objs")
        exclude = config.get("compile", "exclude")

    tests = glob('*.mod')
    res = do_tests(tests)
    os.chdir("..")
    return res


def get_tests():
    tests = glob('tests.*')
    return tests


def main():

    argsParser = argparse.ArgumentParser()
    argsParser.add_argument(
        '-t', '--tests', help="run test on these directories")
    args = argsParser.parse_args()

    res = 0
    if args.tests:
        x = args.tests
        tests = x.split(",")
    else:
        tests = get_tests()

    print(f"tests {tests}")
    for d in tests:
        res = res + do_tests_dir(d)
    if res > 0:
        print(red)
    print(f"### Total fails {res}")
    print(restore)
    return res


if __name__ == '__main__':
    exit(main())
