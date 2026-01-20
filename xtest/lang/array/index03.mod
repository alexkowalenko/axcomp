MODULE index03;
VAR 
    a : ARRAY 3 OF INTEGER;
    x : INTEGER;
BEGIN
    a[0] := 7;
    a[1] := 3;
    a[2] := 5;
    
    x := a[0] + a[1] + a[2];
    WriteInt(x); WriteLn;
END index03.

(*
RUN: %comp %s | filecheck %s
CHECK: 15
CHECK-NEXT: 0
*)