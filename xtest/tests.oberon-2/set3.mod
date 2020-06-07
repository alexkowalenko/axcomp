MODULE set3; (* SET *)
IMPORT Out;
VAR x, y, z: SET;
   a: INTEGER;
BEGIN
   a := 3;
   z := {a};
   INCL(z, 4);

   IF 3 IN z THEN
      Out.Int(3, 0); Out.String(" in set x"); Out.Ln;
   END;
   IF 4 IN z THEN
      Out.Int(4, 0); Out.String(" in set x"); Out.Ln;
   END;
   Out.Ln;

   EXCL(z, 4);
   IF 4 IN z THEN
      Out.Int(4, 0); Out.String(" in set x"); Out.Ln;
   END;
   IF 3 IN z THEN
      Out.Int(3, 0); Out.String(" in set x"); Out.Ln;
   END;


   Out.String("Finish!"); Out.Ln;
END set3.