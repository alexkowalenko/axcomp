MODULE prime4e;
VAR n: INTEGER;

PROCEDURE Primes(n: INTEGER);
    VAR k, lim: INTEGER; prim: BOOLEAN;
BEGIN
    k := 2; lim := 3; prim := TRUE;
    WHILE prim DO
        k := k + 1;
        prim := FALSE
    END
END Primes;

BEGIN
    n := 283;
    Primes(n)
END prime4e.

(\*
RUN: %comp %s | filecheck %s
CHECK: 0
*\)
