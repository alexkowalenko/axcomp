MODULE Doors; (* From rosettacode.org *)
  IMPORT Out;

  CONST N = 100; len = 101;
  VAR i, j: INTEGER;
      closed: ARRAY 101 OF BOOLEAN;  (* Arrays in Oberon always start with index 0; closed[0] is not used *)
  
BEGIN
    FOR i := 1 TO N DO closed[i] := TRUE END;
    FOR i := 1 TO N DO
      j := 1;
      WHILE j < len DO
        IF j MOD i = 0 THEN closed[j] := ~closed[j] END; j := j + 1  (* ~ = NOT *)
      END
    END;
    (* Print a state diagram of all doors *)
    FOR i := 1 TO N DO 
      IF (i - 1) MOD 10 = 0 THEN Out.Ln END;
      IF closed[i] THEN Out.Char('-') Out.Char(' ')  ELSE Out.Char('+') Out.Char(' ') END
    END;  Out.Ln;
    (* Print the numbers of the open doors *)
    FOR i := 1 TO N DO 
      IF ~closed[i] THEN Out.Int(i, 0); Out.Char(' ') END
    END;  Out.Ln
END Doors.

(*
RUN: %comp %s | filecheck %s
CHECK: + - - + - - - - + -
CHECK-NEXT: - - - - - + - - - -
CHECK-NEXT: - - - - + - - - - -
CHECK-NEXT: - - - - - + - - - -
CHECK-NEXT: - - - - - - - - + -
CHECK-NEXT: - - - - - - - - - -
CHECK-NEXT: - - - + - - - - - -
CHECK-NEXT: - - - - - - - - - -
CHECK-NEXT: + - - - - - - - - -
CHECK-NEXT: - - - - - - - - - +
CHECK-NEXT: 1 4 9 16 25 36 49 64 81 100
CHECK-NEXT: 0
*)
