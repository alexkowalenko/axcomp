#!env python3
#
# AX compiler
#
# Copyright © 2020 Alex Kowalenko
#

from glob import glob
from pathlib import Path
import os
from colored import fg, bg, attr

import argparse

install_dir = "../../bin"

compiler = f"{install_dir}/ax --output_funct "
linker = f"clang++ ../main.cc -L {install_dir} -lAx "

red = fg('red_1')
restore = attr('reset')


def do_clang(stem):
    obj = stem + ".o"
    ret = os.system(linker + obj)
    if (ret != 0):
        print(red + "compile " + restore, end="")
        return 0
    os.system("./a.out > result.txt")
    res = stem + ".res"
    ret = os.system(
        f"diff --strip-trailing-cr {res} result.txt > result.diff.txt")
    if(ret != 0):
        print(red + "run " + restore, end="")
        fail = stem + ".fail"
        os.system(f"mv result.txt {fail}")
    os.system(f"rm -f a.out {obj} result.diff.txt result.txt")
    return (ret == 0)


def do_test(t):
    stem = Path(t).stem
    fail = stem + ".fail"
    cmd = f"{compiler} --file {t} > result.txt"
    # print(cmd)
    ret = os.system(cmd)
    if ret:
        os.system(f"mv result.txt {fail}")
        print(red + "compile " + restore, end="")
        return 0
    exp = stem + ".exp"
    asm = stem + ".ll"
    cmd = f"diff --strip-trailing-cr {exp} {asm} > result.diff.txt"
    # print(cmd)
    ret = os.system(cmd)
    if(ret != 0):
        os.system(f"mv {asm} {fail}")
        os.system(f"rm {stem}.o")
    else:
        os.system(f"rm -f {fail} {asm}")
    os.system("rm -f result.diff.txt")
    if (ret != 0):
        print(red + "llir " + restore, end="")
        return 0
    # compile
    return do_clang(stem)


def do_tests(l):
    count = 0
    fails = 0
    for x in l:
        print(f"  : {x} ", end="")
        result = do_test(x)
        count += 1
        if(not result):
            print(red + f"-> :Fail" + restore, end="")
            fails += 1
        print()
    print(f"Ran {count} tests with {fails} fails.")
    return fails


def do_tests_dir(d):
    print(f"Tests: {d}")
    os.chdir(d)
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
    return res


if __name__ == '__main__':
    exit(main())
