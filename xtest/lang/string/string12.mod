MODULE string12; (* 1 char STRING functions as CHAR *)
IMPORT Out;

PROCEDURE print(a : CHAR);
BEGIN
   Out.Char(a); Out.Ln; 
END print;

PROCEDURE print2(a : STRING);
BEGIN
   Out.String(a); Out.Ln; 
END print2;

BEGIN
  print("C");
  print('c');
  print2("C");
END string12.

(*
RUN: %comp %s | filecheck %s
CHECK: C
CHECK-NEXT: c
CHECK-NEXT: C
CHECK-NEXT: 0
*)