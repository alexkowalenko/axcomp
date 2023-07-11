MODULE set4; (* SET *)
IMPORT Out;
VAR y, z: SET;
   a: INTEGER;
BEGIN
   a := 3;
   z := {a};
   INCL(z, 4);

   y := z;

   IF 3 IN y THEN
      Out.Int(3, 0); Out.String(" in set y"); Out.Ln;
   END;
   IF 4 IN y THEN
      Out.Int(4, 0); Out.String(" in set y"); Out.Ln;
   END;

   Out.String("Finish!"); Out.Ln;
END set4.