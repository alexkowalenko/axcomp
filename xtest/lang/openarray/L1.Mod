MODULE L1;
IMPORT Out;
VAR x : STRING;
PROCEDURE f*(VAR str: ARRAY OF CHAR): LONGINT;
  VAR
    i: LONGINT;
  BEGIN
    i := 0;
    LOOP
      IF (str[i] = 0X) THEN
        EXIT;
      END;
      i := i+1;
    END;
    RETURN i;
  END f;

BEGIN 
  x := "Hello";
  Out.String(x); Out.Char(' '); Out.Int(f(x), 0); Out.Ln;
END L1.

(*
RUN: %comp %s | filecheck %s
CHECK: Hello 5
CHECK: 0
*)
