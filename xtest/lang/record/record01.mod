MODULE record01; (* RECORD with base types *)
IMPORT Out;
  TYPE pt = RECORD
          x: INTEGER;
          y: INTEGER
      END;
      pt3 = RECORD (pt)
          z: INTEGER;
      END;
  VAR a : pt3;
      b : pt;
BEGIN
  a.x := 1;
  a.y := 2;
  a.z := 3;
  b.x := a.x;
  b.y := a.y;
  Out.String("a := {"); Out.Int(a.x, 0); Out.Char(','); Out.Int(a.y, 0); Out.Char(','); Out.Int(a.z, 0); Out.Char('}'); Out.Ln; 
  Out.String("b := {"); Out.Int(b.x, 0); Out.Char(','); Out.Int(b.y, 0); Out.Char('}'); Out.Ln; 
  Out.String("sum a = "); Out.Int(a.x + a.y + a.z, 0); Out.Ln; 
  Out.String("sum b = "); Out.Int(b.x + b.y, 0); Out.Ln; 
  RETURN a.x;
END record01.


(*
RUN: %comp %s | filecheck %s
CHECK: a := {1,2,3}
CHECK-NEXT:       b := {1,2}
CHECK-NEXT:       sum a = 6
CHECK-NEXT:       sum b = 3
CHECK-NEXT: 1
*)
