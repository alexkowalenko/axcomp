MODULE record17; (* RECORDS with VAR *)
IMPORT Out;
TYPE pt = RECORD x, y : INTEGER; END;
VAR x: pt;

PROCEDURE print(a: pt);
BEGIN
     Out.Char('('); Out.Int(a.x, 0); Out.Char(',');  Out.Int(a.y, 0); Out.Char(')'); 
END print;

PROCEDURE clear(VAR a : pt);
BEGIN
    a.x := 0; a.y := 0;
END clear;

PROCEDURE set(VAR a : pt; x, y: INTEGER);
BEGIN
    a.x := x; a.y := y;
END set;

BEGIN
    x.x := 1;
    x.y := 2;
    print(x); Out.Ln;
    clear(x); 
    print(x); Out.Ln;
    set(x, 4, 3);
    print(x); Out.Ln;
END record17.


(*
RUN: %comp %s | filecheck %s
CHECK: (1,2)
CHECK-NEXT:       (0,0)
CHECK-NEXT:       (4,3)
CHECK: 0
*)