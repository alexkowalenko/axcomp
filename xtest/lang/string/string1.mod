MODULE string1; (* STRING literals *)
IMPORT Out;
CONST a = "Hello";
VAR x: STRING;

BEGIN
   x := 'Hello';
   RETURN 0;
END string1.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)