(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)

MODULE Undef3;

IMPORT Out;

PROCEDURE P*(b: BOOLEAN): INTEGER;
  VAR
    x: INTEGER;
  BEGIN
    IF b THEN
      x := 1;
    END;
    RETURN x;
  END P;

BEGIN
  Out.Int(P(TRUE), 0); Out.Ln;
END Undef3.
