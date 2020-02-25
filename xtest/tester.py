#!env python3

from glob import glob
from pathlib import Path
import os
from colored import fg, bg, attr

compiler = "../bin/ax"

red = fg('red_1')
restore = attr('reset')


def do_clang(stem):
    obj = stem + ".o"
    ret = os.system(f"clang main.c {obj}")
    if (ret != 0):
        print(red + "compile " + restore, end="")
        # os.system(f"rm {obj}")
        return 0
    os.system(f"./a.out > result.txt")
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
    cmd = f"{compiler} --file {t}"
    # print(cmd)
    os.system(cmd)
    exp = stem + ".exp"
    asm = stem + ".ll"
    cmd = f"diff --strip-trailing-cr {exp} {asm} > result.diff.txt"
    # print(cmd)
    ret = os.system(cmd)
    fail = stem + ".fail"
    if(ret != 0):
        os.system(f"mv {asm} {fail}")
        os.system(f"rm {stem}.o")
    else:
        os.system(f"rm -f {fail} {asm}")
    os.system("rm -f result.diff.txt")
    if (ret != 0):
        print(red + "compile " + restore, end="")
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
