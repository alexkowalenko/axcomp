MODULE len; (* LEN function*)
IMPORT Out;
VAR i: INTEGER;
    y : ARRAY 7 OF INTEGER;

BEGIN
     Out.Int(LEN(y), 0); Out.Ln;
     FOR i := 0 TO LEN(y) - 1 DO
       y[i] := i*i;
     END;
      FOR i := 0 TO LEN(y) - 1 DO
        Out.Int(y[i], 0); Out.Ln;
     END;
    
END len.

(*
RUN: %comp %s | filecheck %s
CHECK: 7
CHECK-NEXT: 0
*)
