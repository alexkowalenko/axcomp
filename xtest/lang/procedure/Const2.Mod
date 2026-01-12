MODULE Const2;

IMPORT Out;

PROCEDURE Test* ();
  BEGIN
    IF FALSE THEN
      Out.String ("foo");
      RETURN;
    END;
    Out.String ("bar");
    Out.Ln;
  END Test;

BEGIN
  Test
END Const2.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
CHECK-NEXT: 0
*)
