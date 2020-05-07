MODULE real04; (* REAL *)
IMPORT Out, Math;
VAR x : REAL;
BEGIN
    Out.String("pi: "); Out.Real(Math.pi); Out.Ln;
    Out.String("e: "); Out.Real(Math.e); Out.Ln;
    Out.Ln;
    
    Out.String("sin: "); Out.Real(Math.sin(0.0)); Out.Ln;
    Out.String("cos: "); Out.Real(Math.cos(0.0)); Out.Ln;
    Out.String("arctan: "); Out.Real(Math.arctan(1.0)); Out.Ln;

    Out.String("ln: "); Out.Real(Math.ln(Math.e)); Out.Ln;
    Out.String("exp: "); Out.Real(Math.exp(1.0)); Out.Ln;
    Out.String("sqrt: "); Out.Real(Math.sqrt(2.0)); Out.Ln;

    IF Math.pi > Math.e THEN
         Out.String("pi is greater than e");  Out.Ln;
    END
    RETURN 0;
END real04.