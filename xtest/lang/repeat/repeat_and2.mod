MODULE repeat_and2;

PROCEDURE Test;
    VAR prim: BOOLEAN; k, lim: INTEGER;
BEGIN
    prim := TRUE; k := 0; lim := 2;
    REPEAT
        k := k + 1;
        prim := TRUE
    UNTIL prim & (k < lim)
END Test;

BEGIN
    Test
END repeat_and2.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)
