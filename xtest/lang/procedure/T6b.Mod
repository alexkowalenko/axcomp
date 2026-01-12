MODULE T6b;

IMPORT Out;

PROCEDURE f*(a, b, c: BOOLEAN): LONGINT;
  BEGIN
    IF a & (b & c) THEN
      RETURN 9;
    ELSE
      RETURN -9;
    END;
  END f;

BEGIN
  Out.Int(f(TRUE, TRUE, TRUE), 0); Out.Ln;
END T6b.

(*
RUN: %comp %s | filecheck %s
CHECK: 9
CHECK-NEXT: 0
*)
