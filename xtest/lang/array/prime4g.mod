MODULE prime4g;

PROCEDURE Test;
    VAR prim, flag: BOOLEAN; k: INTEGER;
BEGIN
    prim := TRUE; flag := TRUE; k := 0;
    WHILE prim & flag DO
        k := k + 1;
        prim := FALSE
    END
END Test;

BEGIN
    Test
END prime4g.

(\*
RUN: %comp %s | filecheck %s
CHECK: 0
*\)
