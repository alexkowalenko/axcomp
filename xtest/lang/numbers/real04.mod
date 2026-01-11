(*
RUN: %comp %s | filecheck %s
CHECK: pi: 3.14159
CHECK-NEXT:      e: 2.71828
CHECK-EMPTY:
CHECK-NEXT:      sin: 0
CHECK-NEXT:      cos: 1
CHECK-NEXT:      arctan: 0.785398
CHECK-NEXT:      ln: 1
CHECK-NEXT:      exp: 2.71828
CHECK-NEXT:      sqrt: 1.41421
CHECK-NEXT:      pi is greater than e
CHECK-NEXT:      0
*)

MODULE real04; (* REAL *)
IMPORT Out, Math;
VAR x : REAL;
BEGIN
    Out.String("pi: "); Out.Real(Math.pi, 0); Out.Ln;
    Out.String("e: "); Out.Real(Math.e, 0); Out.Ln;
    Out.Ln;
    
    Out.String("sin: "); Out.Real(Math.sin(0.0), 0); Out.Ln;
    Out.String("cos: "); Out.Real(Math.cos(0.0), 0); Out.Ln;
    Out.String("arctan: "); Out.Real(Math.arctan(1.0), 0); Out.Ln;

    Out.String("ln: "); Out.Real(Math.ln(Math.e), 0); Out.Ln;
    Out.String("exp: "); Out.Real(Math.exp(1.0), 0); Out.Ln;
    Out.String("sqrt: "); Out.Real(Math.sqrt(2.0), 0); Out.Ln;

    IF Math.pi > Math.e THEN
         Out.String("pi is greater than e");  Out.Ln;
    END
    RETURN 0;
END real04.