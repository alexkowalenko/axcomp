MODULE string6; (* STRING type *)
IMPORT Out;
VAR   x: STRING;

BEGIN
    x := 'Hello';
   Out.String(x); Out.Ln;
   x[0] := 'B';
   x[1] := 'i';
   Out.String(x); Out.Ln;
   RETURN x[0];
END string6.

(*
RUN: %comp %s | filecheck %s
CHECK: Hello
CHECK-NEXT: Billo
CHECK-NEXT: 66
*)