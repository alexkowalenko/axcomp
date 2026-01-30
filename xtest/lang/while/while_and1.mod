MODULE while_and1;

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
END while_and1.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)
