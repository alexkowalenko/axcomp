MODULE g09; (* Mix ARRAY and RECORD *)

VAR pt : RECORD
        x : INTEGER;
        y : ARRAY 3 OF INTEGER;
END;

BEGIN
    pt.x := 2;
    pt.y[0] := 1;
    pt.y[1] := 2;
    pt.y[2] := 4;
    RETURN pt.x + pt.y[0] + pt.y[1] + pt.y[2]
END g09.

(*
RUN: %comp %s | filecheck %s
CHECK: 9
*)