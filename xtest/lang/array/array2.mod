MODULE array2; (* Multi-dimension arrays *)
IMPORT Out;
VAR x : ARRAY 3, 4, 5 OF INTEGER;
    sum, i, j, k : INTEGER;
BEGIN
    FOR i := 0 TO 2 DO
        FOR j := 0 TO 3 DO
            FOR k := 0 TO 4 DO
                x[i,j, k] := sum;
                INC(sum);
            END
        END
    END
    FOR i := 0 TO 2 DO
        FOR j := 0 TO 3 DO
            FOR k := 0 TO 4 DO
                Out.Int(i, 0) Out.Char(','); Out.Int(j, 0) Out.Char(','); Out.Int(k, 0) Out.Char('='); 
                Out.Int(x[i,j,k], 0) Out.Char(' ');
            END
            Out.Ln;
        END
        Out.Ln;
    END
END array2.

(*
RUN: %comp %s | filecheck %s
CHECK:0,0,0=0 0,0,1=1 0,0,2=2 0,0,3=3 0,0,4=4
CHECK-NEXT:      0,1,0=5 0,1,1=6 0,1,2=7 0,1,3=8 0,1,4=9
CHECK-NEXT:      0,2,0=10 0,2,1=11 0,2,2=12 0,2,3=13 0,2,4=14
CHECK-NEXT:      0,3,0=15 0,3,1=16 0,3,2=17 0,3,3=18 0,3,4=19
CHECK-EMPTY:
CHECK-NEXT:      1,0,0=20 1,0,1=21 1,0,2=22 1,0,3=23 1,0,4=24
CHECK-NEXT:      1,1,0=25 1,1,1=26 1,1,2=27 1,1,3=28 1,1,4=29
CHECK-NEXT:      1,2,0=30 1,2,1=31 1,2,2=32 1,2,3=33 1,2,4=34
CHECK-NEXT:      1,3,0=35 1,3,1=36 1,3,2=37 1,3,3=38 1,3,4=39
CHECK-EMPTY:
CHECK-NEXT:      2,0,0=40 2,0,1=41 2,0,2=42 2,0,3=43 2,0,4=44
CHECK-NEXT:      2,1,0=45 2,1,1=46 2,1,2=47 2,1,3=48 2,1,4=49
CHECK-NEXT:      2,2,0=50 2,2,1=51 2,2,2=52 2,2,3=53 2,2,4=54
CHECK-NEXT:      2,3,0=55 2,3,1=56 2,3,2=57 2,3,3=58 2,3,4=59
CHECK-EMPTY:
CHECK-NEXT: 0
*)