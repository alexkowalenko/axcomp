MODULE string21; (* STRING type *)
IMPORT Out;
VAR x: STRING;

BEGIN
   x := 'Hello';
   Out.String(x); Out.Ln;
   RETURN 0;
END string21.

(*
RUN: %comp %s | filecheck %s
CHECK: Hello
CHECK-NEXT: 0
*)