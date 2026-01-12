MODULE HOTLoop5;

IMPORT Out;

PROCEDURE Test* ();
  VAR
    i, j: INTEGER;
  BEGIN
    i := 1;
    REPEAT
      j := 1;
      REPEAT
        Out.Int (j, 2);
        INC (j);
      UNTIL (j > i);
      Out.Ln;
      INC (i);
    UNTIL (i = 5);
  END Test;

BEGIN
  Test
END HOTLoop5.

(*
RUN: %comp %s | filecheck %s
CHECK: 1
CHECK-NEXT: 1 2
CHECK-NEXT: 1 2 3
CHECK-NEXT: 1 2 3 4
*)


