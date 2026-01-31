MODULE Odd1;

IMPORT Out;

PROCEDURE Odd (x: INTEGER; odd: BOOLEAN);
  BEGIN
    Out.Int(x, 4);
    Out.Bool(odd);
    Out.Bool(odd);
    Out.Ln;
  END Odd;

PROCEDURE Test* ();
  BEGIN
    Odd (65, TRUE);
    Odd (96, FALSE);
    Odd (-1, TRUE);
    Odd (0, FALSE);
  END Test;

BEGIN
  Test
END Odd1.


(*
RUN: %comp %s | filecheck %s
CHECK: 6511
CHECK-NEXT: 9600
CHECK-NEXT: -111
CHECK-NEXT: 000
*)
