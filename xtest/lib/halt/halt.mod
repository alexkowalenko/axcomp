MODULE halt; (* HALT function*)
IMPORT Out;
VAR x, i: INTEGER;

BEGIN
       FOR i := 1 TO 3 DO
              HALT(3);
       END
       RETURN 0;
END halt.

(*
RUN: %comp %s | echo $? | filecheck %s
CHECK: 0
*)
