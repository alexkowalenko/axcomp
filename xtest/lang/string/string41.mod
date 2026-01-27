MODULE string41; (* STRING type *)
IMPORT Out;
VAR
      x : STRING;
      xx: ARRAY 3 OF CHAR;
      c: CHAR;

BEGIN
   xx[0] := 'A';
   Out.Char(xx[0]); Out.Ln;
   x := 'Hello';
   Out.String(x); Out.Ln;
   x[0] := 'B';  (* Strings are mutable *)
   Out.String(x); Out.Ln;
   c := x[1];
   RETURN c;
END string41.

(*
RUN: %comp %s | filecheck %s
CHECK: A
CHECK-NEXT: Hello
CHECK-NEXT: Bello
CHECK-NEXT: 101
*)
