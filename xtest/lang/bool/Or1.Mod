MODULE Or1;

IMPORT Out;

PROCEDURE P (c: CHAR; x: BOOLEAN): BOOLEAN;
  BEGIN
    Out.Char (c); RETURN x;
  END P;

PROCEDURE Test* ();
  BEGIN
    Out.String ("FALSE OR FALSE: ");
    Out.Bool (P("a", FALSE) OR P("b", FALSE));
    Out.Ln;
    
    Out.String ("FALSE OR TRUE : ");
    Out.Bool (P("a", FALSE) OR P("b", TRUE));
    Out.Ln;
    
    Out.String ("TRUE OR FALSE : ");
    Out.Bool (P("a", TRUE) OR P("b", FALSE));
    Out.Ln;
    
    Out.String ("TRUE OR TRUE  : ");
    Out.Bool (P("a", TRUE) OR P("b", TRUE));
    Out.Ln;
  END Test;

BEGIN
  Test
END Or1.

(*
RUN: %comp %s | filecheck %s
CHECK: FALSE OR FALSE: ab0
CHECK-NEXT: FALSE OR TRUE : ab1
CHECK-NEXT: TRUE OR FALSE : a1
CHECK-NEXT: TRUE OR TRUE  : a1
CHECK-NEXT: 0
*)
