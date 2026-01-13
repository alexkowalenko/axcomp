MODULE For6;

PROCEDURE Test*;
  VAR
    x: SHORTINT;
  BEGIN
    FOR x := 0 TO 127 BY 128 DO
       WriteInt(x); WriteLn;
    END;
  END Test;

BEGIN
  Test
END For6.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
CHECK-NEXT: 0
*)
