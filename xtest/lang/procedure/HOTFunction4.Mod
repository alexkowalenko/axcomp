MODULE HOTFunction4;

IMPORT Out;
  
PROCEDURE Select (b: BOOLEAN): INTEGER;
  BEGIN
    IF b THEN
      RETURN 1
    ELSE
      RETURN 0
    END;
  END Select;

PROCEDURE Test* ();
  BEGIN
    Out.Int(Select(FALSE), 0); Out.Ln;
    Out.Int(Select(TRUE), 0); Out.Ln;
  END Test;

BEGIN
  Test
END HOTFunction4.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
CHECK-NEXT: 1
CHECK-NEXT: 0
*)
