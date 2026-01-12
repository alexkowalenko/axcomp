MODULE Const3;

IMPORT Out;

PROCEDURE P (b: BOOLEAN);
  BEGIN
    LOOP
      IF FALSE THEN
        Out.String ("foo");
        EXIT;
      ELSIF b THEN
        Out.String ("bar");
        EXIT;
      END;
    END;
  END P;

PROCEDURE Test* ();
  BEGIN
    P(TRUE);
    Out.Ln;
  END Test;

BEGIN
  Test
END Const3.

(*
RUN: %comp %s | filecheck %s
CHECK: bar
CHECK-NEXT: 0
*)
