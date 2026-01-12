MODULE T8c;

IMPORT Out;

PROCEDURE f*(a, b, c: BOOLEAN): LONGINT;
  BEGIN
    IF (a OR b) & c THEN
      RETURN 9;
    ELSE
      RETURN -9;
    END;
  END f;

BEGIN
  Out.Int(f(FALSE, TRUE, TRUE), 0); Out.Ln;
END T8c.

(*
RUN: %comp %s | filecheck %s
CHECK: 9
CHECK-NEXT: 0
*)
