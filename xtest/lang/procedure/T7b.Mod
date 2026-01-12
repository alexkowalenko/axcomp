MODULE T7b;

IMPORT Out;

PROCEDURE f*(a, b, c: BOOLEAN): LONGINT;
  BEGIN
    IF a OR (b OR c) THEN
      RETURN 9;
    ELSE
      RETURN -9;
    END;
  END f;

BEGIN
  Out.Int(f(FALSE, TRUE, TRUE), 0); Out.Ln;
END T7b.

(*
RUN: %comp %s | filecheck %s
CHECK: 9
CHECK-NEXT: 0
*)
