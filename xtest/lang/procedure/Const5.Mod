MODULE Const5;

IMPORT Out;

(* <*CaseSelectCheck:=TRUE*> *)
PROCEDURE Test* ();
  BEGIN
    CASE 10 OF
    | 0..2:
      Out.String ("foo");
      RETURN;
    ELSE
    END;
    Out.String ("bar");
    Out.Ln;
  END Test;

BEGIN
  Test
END Const5.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
CHECK-NEXT: 0
*)
