MODULE char; (* Out.String() *)
IMPORT Out;

VAR s : POINTER TO CHAR;

BEGIN
   NEW(s);
   s^ := 'H'; 
   Out.Char(s^); Out.Ln;
END char.

(*
RUN: %comp %s | filecheck %s
CHECK: H
CHECK-NEXT: 0
*)
