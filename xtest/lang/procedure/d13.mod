MODULE d13; (* Test VAR args *)

VAR a : INTEGER;

PROCEDURE f(x : INTEGER);
BEGIN
    WriteInt(x); WriteLn();
    x := 10;
    WriteInt(x); WriteLn()
END f;

BEGIN
    a := 1;
    f(a);
    WriteInt(a); WriteLn()
END d13.

(*
1
10
1
output: 0
*)

(*
RUN: %comp %s | filecheck %s
CHECK: 1
CHECK-NEXT: 10
CHECK-NEXT: 1
CHECK-NEXT: 0
*)
