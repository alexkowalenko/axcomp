MODULE long; (* LONG *)
IMPORT Out;
BEGIN
   Out.Int(LONG(-17), 0); Out.Ln;
   Out.Real(LONG(3.1), 0); Out.Ln; Out.Ln;

   Out.Int(SHORT(-17), 0); Out.Ln;
   Out.Real(SHORT(3.1), 0); Out.Ln;
END long.