MODULE Div1;

IMPORT Out;

PROCEDURE Test* ();
  VAR
    i: INTEGER;
    x: INTEGER;
  BEGIN
    FOR i := -6 TO 6 DO
      Out.Int (i, 3); Out.String (" DIV "); Out.Int (3, 0); Out.String (" = ");
      x := i DIV 3;
      Out.Int (x, 0); Out.Ln;
    END;
  END Test;

BEGIN
  Test
END Div1.

(*
RUN: %comp %s | filecheck %s
CHECK: -6 DIV 3 = -2
CHECK-NEXT:        -5 DIV 3 = -1
CHECK-NEXT:        -4 DIV 3 = -1
CHECK-NEXT:        -3 DIV 3 = -1
CHECK-NEXT:        -2 DIV 3 = 0
CHECK-NEXT:        -1 DIV 3 = 0
CHECK-NEXT:         0 DIV 3 = 0
CHECK-NEXT:         1 DIV 3 = 0
CHECK-NEXT:         2 DIV 3 = 0
CHECK-NEXT:         3 DIV 3 = 1
CHECK-NEXT:         4 DIV 3 = 1
CHECK-NEXT:         5 DIV 3 = 1
CHECK-NEXT:         6 DIV 3 = 2
CHECK-NEXT: 0
*)
