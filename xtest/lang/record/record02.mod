MODULE record02; (* RECORD base types *)
IMPORT Out;
  TYPE pt = RECORD
          x: INTEGER;
          y: INTEGER
      END;
      pt3 = RECORD (pt)
          z: INTEGER;
      END;
  VAR a: pt3;

PROCEDURE print(a : pt3);
BEGIN
   Out.Char('{'); Out.Int(a.x, 0); Out.Char(','); Out.Int(a.y, 0); Out.Char(','); Out.Int(a.z, 0); Out.Char('}'); Out.Ln; 
END print;

PROCEDURE print2(a : pt);
BEGIN
   Out.Char('{'); Out.Int(a.x, 0); Out.Char(','); Out.Int(a.y, 0); Out.Char('}'); Out.Ln; 
END print2;


BEGIN
  a.x := 1;
  a.y := 2;
  a.z := 3;
  
  Out.String("a := "); print(a);
  Out.Ln;

  a.x := 4;
  Out.String("a := "); print(a);
  Out.String("sum a = "); Out.Int(a.x + a.y + a.z, 0); Out.Ln; 
  Out.Ln;

  RETURN a.x;
END record02.


(*
RUN: %comp %s | filecheck %s
CHECK: a := {1,2,3}
CHECK: a := {4,2,3}
CHECK: sum a = 9
CHECK: 4
*)