MODULE prime4f;
VAR n: INTEGER;

PROCEDURE Primes(n: INTEGER);
    VAR k, lim: INTEGER; prim: BOOLEAN;
BEGIN
    k := 2; lim := 3; prim := TRUE;
    WHILE k < lim DO
        k := k + 1;
        prim := FALSE
    END
END Primes;

BEGIN
    n := 283;
    Primes(n)
END prime4f.

(\*
RUN: %comp %s | filecheck %s
CHECK: 0
*\)
