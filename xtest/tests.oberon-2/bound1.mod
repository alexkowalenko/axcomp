MODULE bound1; (* Procedures with recievers *)
IMPORT Out;
TYPE pt = RECORD x, y : INTEGER; END;
VAR x: pt;

PROCEDURE (a : pt) print();
BEGIN
    Out.Char('('); Out.Int(a.x, 0); Out.Char(',');  Out.Int(a.y, 0); Out.Char(')'); 
END print;

PROCEDURE (VAR a : pt) clear();
BEGIN
    a.x := 0; a.y := 0;
END clear;

PROCEDURE (VAR a : pt) set(x, y: INTEGER);
BEGIN
    a.x := x; a.y := y;
END set;

BEGIN
    x.x := 1;
    x.y := 2;
    x.print; Out.Ln;
    x.clear();
    x.print(); Out.Ln;
    x.set(4, 3);
    x.print(); Out.Ln;
END bound1.