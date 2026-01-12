MODULE And1;

IMPORT Out;

PROCEDURE P (c: CHAR; x: BOOLEAN): BOOLEAN;
  BEGIN
    Out.Char (c); RETURN x;
  END P;

PROCEDURE Test* ();
  BEGIN
    Out.String ("FALSE & FALSE: ");
    Out.Bool (P("a", FALSE) & P("b", FALSE));
    Out.Ln;
    
    Out.String ("FALSE & TRUE : ");
    Out.Bool (P("a", FALSE) & P("b", TRUE));
    Out.Ln;
    
    Out.String ("TRUE & FALSE : ");
    Out.Bool (P("a", TRUE) & P("b", FALSE));
    Out.Ln;
    
    Out.String ("TRUE & TRUE  : ");
    Out.Bool (P("a", TRUE) & P("b", TRUE));
    Out.Ln;
  END Test;

BEGIN
  Test
END And1.

(*
RUN: %comp %s | filecheck %s
CHECK: FALSE & FALSE: a0
CHECK-NEXT: FALSE & TRUE : a0
CHECK-NEXT: TRUE & FALSE : ab0
CHECK-NEXT: TRUE & TRUE  : ab1
*)
