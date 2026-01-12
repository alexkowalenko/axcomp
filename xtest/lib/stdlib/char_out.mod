MODULE char_out; (* Out.String() *)
IMPORT Out;
VAR x, y: CHAR;

BEGIN
   x := 'H';
   Out.Char(x); Out.Ln;
   RETURN 0;
END char_out.

(*
RUN: %comp %s | filecheck %s
CHECK: H
CHECK-NEXT: 0
*)
