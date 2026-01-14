MODULE ash; (* ASH function*)
IMPORT Out;
VAR i, x: INTEGER;

BEGIN
       FOR i := 0 TO 7 DO
              x := ASH(1, i);
              Out.Int(x, 0); Out.Ln;
       END
END ash.

(*
RUN: %comp %s | filecheck %s
CHECK: 1
CHECK-NEXT: 2
CHECK-NEXT: 4
CHECK-NEXT: 8
CHECK-NEXT: 16
CHECK-NEXT: 32
CHECK-NEXT: 64
CHECK-NEXT: 128
CHECK-NEXT: 0
*)
