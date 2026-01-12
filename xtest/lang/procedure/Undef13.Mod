MODULE Undef13;

IMPORT Out;

PROCEDURE Fct*(b: BOOLEAN): BOOLEAN;
  BEGIN
    IF b THEN
      RETURN FALSE;
    END;
  END Fct;

BEGIN
  Out.Bool(Fct(TRUE)); Out.Ln;
END Undef13.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
CHECK-NEXT: 0
*)
