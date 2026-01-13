MODULE string; (* Out.String() *)
IMPORT Out;

BEGIN
   Out.String('Hello'); Out.Ln;
END string.

(*
RUN: %comp %s | filecheck %s
CHECK: Hello
*)
