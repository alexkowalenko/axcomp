MODULE Const8;

IMPORT Out;
  
PROCEDURE Tand (b: BOOLEAN);
  BEGIN
    IF TRUE & b THEN
      Out.String ("TRUE ");
    ELSE
      Out.String ("FALSE ");
    END;
  END Tand;

PROCEDURE Fand (b: BOOLEAN);
  BEGIN
    IF FALSE & b THEN
      Out.String ("TRUE ");
    ELSE
      Out.String ("FALSE ");
    END;
  END Fand;

PROCEDURE Tor (b: BOOLEAN);
  BEGIN
    IF TRUE OR b THEN
      Out.String ("TRUE ");
    ELSE
      Out.String ("FALSE ");
    END;
  END Tor;

PROCEDURE For (b: BOOLEAN);
  BEGIN
    IF FALSE OR b THEN
      Out.String ("TRUE ");
    ELSE
      Out.String ("FALSE ");
    END;
  END For;

PROCEDURE Test* ();
  BEGIN
    Tand(TRUE); Tand(FALSE); Out.Ln;
    Fand(TRUE); Fand(FALSE); Out.Ln;
    Tor(TRUE); Tor(FALSE); Out.Ln;
    For(TRUE); For(FALSE); Out.Ln;
  END Test;

BEGIN
  Test;
END Const8.

(*
RUN: %comp %s | filecheck %s
CHECK: TRUE FALSE
CHECK-NEXT: FALSE FALSE
CHECK-NEXT: TRUE TRUE
CHECK-NEXT: TRUE FALSE
CHECK-NEXT: 0
*)
