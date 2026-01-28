MODULE openarray; (* open arrays *)
IMPORT Out;
VAR x : ARRAY OF INTEGER;
    sum, i: INTEGER;
BEGIN
    NEW(x, 10);

    FOR i := 0 TO 9 DO
        x[i] := i*i + i + 1;
    END
    RETURN 0;
END openarray.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)
