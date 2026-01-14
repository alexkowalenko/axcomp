MODULE inc_2; (* INC *)
IMPORT Out;
VAR x: INTEGER;
BEGIN
   x := 0;

   INC(x);
   INC(x, 3);
   INC(x, x);
   Out.Int(x, 0); Out.Ln;
   
   DEC(x);
   DEC(x, 3);
   DEC(x, x);
   Out.Int(x, 0); Out.Ln;

END inc_2.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
CHECK-NEXT: 0
*)
