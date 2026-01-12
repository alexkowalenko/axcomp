MODULE T7;

IMPORT Out;

PROCEDURE f*(a, b: BOOLEAN): LONGINT;
  BEGIN
    IF a OR b THEN
      RETURN 9;
    ELSE
      RETURN -9;
    END;
  END f;

BEGIN
  Out.Int(f(FALSE, TRUE), 0); Out.Ln;
END T7.

(*
RUN: %comp %s | filecheck %s
CHECK: 9
CHECK-NEXT: 0
*)
