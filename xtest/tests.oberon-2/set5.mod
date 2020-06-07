MODULE set5; (* SET *)
IMPORT Out;
VAR x, y, z: SET;
BEGIN
   x := {3};
   y := {4};

   Out.String("Union"); Out.Ln;
   z := x + y; (* union *)

   IF 3 IN z THEN
      Out.Int(3, 0); Out.String(" in set z"); Out.Ln;
   END;
   IF 4 IN z THEN
      Out.Int(4, 0); Out.String(" in set z"); Out.Ln;
   END;
   Out.Ln;

   x := {1..2};
   y := {2..3};

   Out.String("Intersection"); Out.Ln;
   z := x * y; (* intersection *)
   IF 1 IN z THEN
      Out.Int(1, 0); Out.String(" in set z"); Out.Ln;
   END;
   IF 2 IN z THEN
      Out.Int(2, 0); Out.String(" in set z"); Out.Ln;
   END;
   IF 3 IN z THEN
      Out.Int(3, 0); Out.String(" in set z"); Out.Ln;
   END;
   Out.Ln;

   Out.String("Symmetric difference"); Out.Ln;
   z := x / y;
   IF 1 IN z THEN
      Out.Int(1, 0); Out.String(" in set z"); Out.Ln;
   END;
   IF 2 IN z THEN
      Out.Int(2, 0); Out.String(" in set z"); Out.Ln;
   END;
   IF 3 IN z THEN
      Out.Int(3, 0); Out.String(" in set z"); Out.Ln;
   END;
   Out.Ln;

   Out.String("Difference"); Out.Ln;
   z := x - y;
   IF 1 IN z THEN
      Out.Int(1, 0); Out.String(" in set z"); Out.Ln;
   END;
   IF 2 IN z THEN
      Out.Int(2, 0); Out.String(" in set z"); Out.Ln;
   END;
   IF 3 IN z THEN
      Out.Int(3, 0); Out.String(" in set z"); Out.Ln;
   END;
   Out.Ln;

   Out.String("Finish!"); Out.Ln;
END set5.