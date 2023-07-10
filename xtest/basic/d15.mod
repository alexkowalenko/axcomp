MODULE d15; (* Test VAR args *)

VAR a, b : INTEGER;

PROCEDURE h(x : INTEGER; VAR y : INTEGER);
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
    WriteInt(a); WriteLn()
    WriteInt(b); WriteLn() (* should be 20 *)
END d15.

(*
1
2
10
20
1
20
output: 0
*)