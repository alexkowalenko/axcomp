#!env python3

from glob import glob
from pathlib import Path
import os
from colored import fg, bg, attr

compiler = "../bin/ax"

red = fg('red_1')
restore = attr('reset')


def do_clang(stem):
    cmd = f"clang main.c output.o"
    os.system(cmd)
    cmd = f"./a.out > result.txt"
    os.system(cmd)
    res = stem + ".res"
    cmd = f"diff --strip-trailing-cr {res} result.txt > result.diff.txt"
    ret = os.system(cmd)
    if(ret != 0):
        cmd = f"mv result.txt {fail}"
        os.system(cmd)
    cmd = "rm -f a.out output.o result.diff.txt result.txt"
    os.system(cmd)
    return (ret == 0)


def do_test(t):
    stem = Path(t).stem
    cmd = f"{compiler} < {t}"
    # print(cmd)
    os.system(cmd)
    exp = stem + ".exp"
    cmd = f"diff --strip-trailing-cr {exp} output.ll > result.diff.txt"
    # print(cmd)
    ret = os.system(cmd)
    fail = stem + ".fail"
    if(ret != 0):
        cmd = f"mv output.ll {fail}"
        os.system(cmd)
    else:
        cmd = f"rm -f {fail} output.ll"
        os.system(cmd)
    cmd = "rm -f result.diff.txt"
    os.system(cmd)
    if (ret != 0):
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


def get_tests():
    tests = glob('*.mod')
    return tests


def main():
    tests = get_tests()
    return do_tests(tests)


if __name__ == '__main__':
    exit(main())
