MODULE stdlib01;  (* Test the stand library output routines *)

IMPORT Out;
CONST x = 12;
VAR
    s: SET;
    f: LONGREAL;

BEGIN
    Out.Open;
    Out.Int(x, 0); Out.Ln;
    Out.Int(0cafebabeH, 0); Out.Ln;
    Out.Hex(0cafebabeH, 0); Out.Ln;
    Out.Bool(TRUE); Out.Ln;
    s := {4,35}
    Out.Set(s); Out.Ln;
    s := {}
    Out.Set(s); Out.Ln;
    f := 3.14159;
    Out.LongReal(f, 3); Out.Ln;
    Out.Flush;
END stdlib01.