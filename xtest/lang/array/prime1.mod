MODULE prime1;
VAR n: INTEGER;
    p: ARRAY 400 OF INTEGER;
    v: ARRAY 20 OF INTEGER;

PROCEDURE Primes(n: INTEGER);
    VAR i, k, m, x, inc, lim, sqr: INTEGER; prim: BOOLEAN;
BEGIN
    x := 1; inc := 4; lim := 1; sqr := 4; m := 0; i := 3;
END Primes;

BEGIN
    n := 283;
    Primes(n)
END prime1.

(\*
RUN: %comp %s | filecheck %s
CHECK: 0
*\)
