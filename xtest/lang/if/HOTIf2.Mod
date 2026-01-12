MODULE HOTIf2;

IMPORT Out;
  
PROCEDURE If (b: BOOLEAN);
  VAR
    x: INTEGER;
  BEGIN
    IF b THEN
      Out.String ("TRUE ")
    END;
    Out.String ("EXIT");
    Out.Ln
  END If;

PROCEDURE Test* ();
  BEGIN
    If (TRUE);
    If (FALSE);
  END Test;

BEGIN
  Test
END HOTIf2.

(*
RUN: %comp %s | filecheck %s
CHECK: TRUE EXIT
CHECK-NEXT: EXIT
CHECK-NEXT: 0
*)
