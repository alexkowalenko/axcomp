MODULE g03; (* Arrays *)

VAR x: ARRAY 3 OF INTEGER;
    i: INTEGER;

BEGIN
    x[0] := 1;
    x[1] := 2;
    x[2] := 3;
    FOR i := 0 TO 2 DO
        WriteInt(x[i]); WriteLn()
    END;
    RETURN x[0] + x[1] + x[2]
END g03.

(*
RUN: %comp %s | filecheck %s
CHECK: 1
CHECK-NEXT: 2
CHECK-NEXT: 3
CHECK-NEXT: 6
*)