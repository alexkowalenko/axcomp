MODULE g04; (* Arrays *)

VAR x: ARRAY 3 OF BOOLEAN;
    i: INTEGER;

BEGIN
    x[0] := FALSE;
    x[1] := TRUE;
    x[2] := FALSE;
    FOR i := 0 TO 2 DO
        WriteBoolean(x[i]); WriteLn()
    END;
    RETURN x[0] OR x[1] OR x[2]
END g04.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
CHECK-NEXT: 1
CHECK-NEXT: 0
CHECK-NEXT: 1
*)