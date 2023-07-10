MODULE bound2; (* Procedures with recievers *)
IMPORT Out;

TYPE    pt = RECORD x, y : INTEGER; END;
        pt3 = RECORD(pt) z: INTEGER; END;

VAR i: pt;
    j: pt3;

PROCEDURE (a : pt) print();
BEGIN
    Out.Char('('); Out.Int(a.x, 0); Out.Char(',');  Out.Int(a.y, 0); Out.Char(')'); 
END print;

PROCEDURE (VAR a : pt) set(x, y: INTEGER);
BEGIN
    a.x := x; a.y := y;
END set;


BEGIN
    i.set(3,4);
    i.print; Out.Ln;
    j.set(4,3);
    j.z := 5;
    j.print; Out.Ln;
    Out.Int(j.z, 0); Out.Ln;
END bound2.