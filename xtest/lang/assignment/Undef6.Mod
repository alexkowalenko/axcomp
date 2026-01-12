(*
RUN: %comp %s | filecheck %s
CHECK: 1
CHECK-NEXT: 0
*)

MODULE Undef6;

IMPORT Out;

PROCEDURE P*(b: BOOLEAN): INTEGER;
  VAR
    x: INTEGER;
  BEGIN
    LOOP
      IF b THEN
        x := 1;
        EXIT;
      ELSE
        EXIT;
      END;
    END;
    RETURN x;
  END P;

BEGIN
  Out.Int(P(TRUE), 0); Out.Ln;
END Undef6.
