MODULE For9;

IMPORT Out;
  
PROCEDURE Test*;
  VAR
    x: INTEGER;
  BEGIN
    FOR x := 1 TO 8 BY 2 DO
      Out.Int (x, 0); Out.Ln;
    END;
  END Test;
  
BEGIN
  Test
END For9.

(*
RUN: %comp %s | filecheck %s
CHECK: 1
CHECK-NEXT: 3
CHECK-NEXT: 5
CHECK-NEXT: 7
CHECK-NEXT: 0
*)
