MODULE d14; (* Test VAR args *)

VAR a : INTEGER;

PROCEDURE g(VAR x : INTEGER);
BEGIN
    WriteInt(x); WriteLn();
    x := 10;
    WriteInt(x); WriteLn()
END g;

BEGIN
    a := 2;
    g(a)
    WriteInt(a); WriteLn() (* should be 10 *)
END d14.

(*
2
10
10
output: 0
*)

(*
RUN: %comp %s | filecheck %s
CHECK: 2
CHECK-NEXT: 10
CHECK-NEXT: 10
CHECK-NEXT: 0
*)
