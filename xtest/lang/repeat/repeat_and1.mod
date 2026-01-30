MODULE repeat_and1;

PROCEDURE Test;
    VAR prim, flag: BOOLEAN; k: INTEGER;
BEGIN
    prim := TRUE; flag := TRUE; k := 0;
    REPEAT
        k := k + 1;
        prim := TRUE
    UNTIL prim & flag
END Test;

BEGIN
    Test
END repeat_and1.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)
