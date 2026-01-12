MODULE e09; (* WHILE block *)

VAR x : INTEGER;

BEGIN
    x := 0;
    WHILE x < 10 DO
        x := x + 1
    END;
    RETURN x
END e09.

(*
RUN: %comp %s | filecheck %s
CHECK: 10
*)

