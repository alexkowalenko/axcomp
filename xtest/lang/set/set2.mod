MODULE set2; (* SET *)
IMPORT Out;
VAR x, y, z: SET;
   a: INTEGER;
BEGIN
   a := 3;
   z := {a};
   IF a IN z THEN
      Out.Int(a, 0); Out.String(" in set x"); Out.Ln;
   END;
   IF 1 IN z THEN
      Out.Int(1, 0); Out.String(" in set x"); Out.Ln;
   END;

   y := {7..17};
   IF 7 IN y THEN
      Out.Int(7, 0); Out.String(" in set y"); Out.Ln;
   END;

   x := {3};
   IF x = z THEN
      Out.String("x = z"); Out.Ln;
   END;

   IF x # z THEN
     Out.String("x # z"); Out.Ln;
   END;

   IF y # z THEN
     Out.String("y # z"); Out.Ln;
   END;

   Out.String("Finish!"); Out.Ln;
END set2.

(*
RUN: %comp %s | filecheck %s
CHECK: 3 in set x
CHECK-NEXT: 7 in set y
CHECK-NEXT: x = z
CHECK-NEXT: y # z
CHECK-NEXT: Finish!
CHECK-NEXT: 0
*)
