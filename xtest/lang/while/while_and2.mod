MODULE while_and2;

PROCEDURE Test;
    VAR prim: BOOLEAN; k, lim: INTEGER;
BEGIN
    prim := TRUE; k := 0; lim := 2;
    WHILE prim & (k < lim) DO
        k := k + 1;
        prim := FALSE
    END
END Test;

BEGIN
    Test
END while_and2.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)
