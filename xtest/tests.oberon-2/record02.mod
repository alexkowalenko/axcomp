MODULE record02; (* RECORD base types *)
IMPORT Out;
  TYPE pt = RECORD
          x: INTEGER;
          y: INTEGER
      END;
      pt3 = RECORD (pt)
          z: INTEGER;
      END;
  VAR a, aa : pt3;
      b : pt;

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
  aa := a;
  
  Out.String("a := "); print(a);
  Out.String("aa := "); print(aa);
  Out.Ln;

  a.x := 4;
  Out.String("a := "); print(a);
  Out.String("aa := "); print(aa);
  Out.String("sum a = "); Out.Int(a.x + a.y + a.z, 0); Out.Ln; 
  Out.Ln;
  
  b.x := 7;
  a := b;
  Out.String("a := "); print(a);

  RETURN a.x;
END record02.