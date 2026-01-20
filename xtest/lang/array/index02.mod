MODULE index02;
VAR 
    a : ARRAY 3 OF INTEGER;
    x : INTEGER;
BEGIN
    a[0] := 7;
    x := a[0];
    WriteInt(x); WriteLn;
END index02.

(*
RUN: %comp %s | filecheck %s
CHECK: 7
CHECK-NEXT: 0
*)