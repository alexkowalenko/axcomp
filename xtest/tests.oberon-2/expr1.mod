MODULE expr1; (* comparison mixed numerics and CHAR *)
IMPORT Out, Math;
BEGIN
    Out.Bool(1 < 2); Out.Ln;
    Out.Bool(1 >= 2.0); Out.Ln;
    Out.Bool(1 < Math.pi); Out.Ln;
    Out.Bool('a' >= 'b'); Out.Ln;
    RETURN 0
END expr1.