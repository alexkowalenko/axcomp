MODULE prime4d;
VAR n: INTEGER;
    p: ARRAY 400 OF INTEGER;
    v: ARRAY 20 OF INTEGER;

PROCEDURE Primes(n: INTEGER);
    VAR i, k, m, x, inc, lim, sqr: INTEGER; prim: BOOLEAN;
BEGIN
    x := 1; inc := 4; lim := 1; sqr := 4; m := 0; i := 3;
    k := 2; prim := TRUE;
    WHILE prim & (k < lim) DO
        k := k + 1;
        prim := FALSE
    END
END Primes;

BEGIN
    n := 283;
    Primes(n)
END prime4d.

(\*
RUN: %comp %s | filecheck %s
CHECK: 0
*\)
