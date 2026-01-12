MODULE set1; (* SET *)
IMPORT Out;
VAR x, y, z: SET;
BEGIN
   x := {0,1,2};
   y := {4};
   z := {};
   Out.Int(MIN(SET), 0); Out.String(" - "); Out.Int(MAX(SET), 0); Out.Ln;
END set1.

(*
RUN: %comp %s | filecheck %s
CHECK: 0 - 63
CHECK-NEXT: 0
*)
