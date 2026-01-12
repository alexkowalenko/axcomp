MODULE d16; (* Test VAR args *)

VAR a, b : INTEGER;

PROCEDURE h(VAR x, y : INTEGER);
BEGIN
    WriteInt(x); WriteLn();
    WriteInt(y); WriteLn();
    x := 10;
    y := 20;
    WriteInt(x); WriteLn();
    WriteInt(y); WriteLn()
END h;

BEGIN
    a := 1;
    b := 2;
    h(a, b);
    WriteInt(a); WriteLn() (* should be 10 *)
    WriteInt(b); WriteLn() (* should be 20 *)
END d16.

(*
1
2
10
20
10
20
output: 0
*)

(*
RUN: %comp %s | filecheck %s
CHECK: 1
CHECK-NEXT: 2
CHECK-NEXT: 10
CHECK-NEXT: 20
CHECK-NEXT: 10
CHECK-NEXT: 20
CHECK-NEXT: 0
*)
