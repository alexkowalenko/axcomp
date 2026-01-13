MODULE Mod2;

IMPORT Out;

PROCEDURE Mod (i, j: INTEGER);
  BEGIN
    Out.Int (i, 3); Out.String (" MOD "); Out.Int (j, 0); Out.String (" = ");
    Out.Int (i MOD j, 0); Out.Ln;
  END Mod;

PROCEDURE Test* ();
  VAR
    i: INTEGER;
  BEGIN
    FOR i := -6 TO 6 DO
      Mod (i, -3);
    END;
  END Test;

BEGIN
  Test
END Mod2.

(*
RUN: %comp %s | filecheck %s
CHECK: -6 MOD -3 = 0
CHECK-NEXT: -5 MOD -3 = -2
CHECK-NEXT: -4 MOD -3 = -1
CHECK-NEXT: -3 MOD -3 = 0
CHECK-NEXT: -2 MOD -3 = -2
CHECK-NEXT: -1 MOD -3 = -1
CHECK-NEXT:  0 MOD -3 = 0
CHECK-NEXT:  1 MOD -3 = 1
CHECK-NEXT:  2 MOD -3 = 2
CHECK-NEXT:  3 MOD -3 = 0
CHECK-NEXT:  4 MOD -3 = 1
CHECK-NEXT:  5 MOD -3 = 2
CHECK-NEXT:  6 MOD -3 = 0
CHECK-NEXT: 0
*)
