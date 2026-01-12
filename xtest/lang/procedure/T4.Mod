MODULE T4;

IMPORT Out;

PROCEDURE f*(select: BOOLEAN; a, b: LONGINT): LONGINT;
  VAR
    x: LONGINT;
  BEGIN
    IF select THEN
      x := a+b;
    ELSE
      x := a-b;
    END;
    RETURN x;
  END f;

BEGIN
  Out.Int(f(TRUE, 3, 17), 0); Out.Ln;
END T4.

(*
RUN: %comp %s | filecheck %s
CHECK: 20
CHECK-NEXT: 0
*)
