(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

MODULE shadow;

VAR x : INTEGER;

PROCEDURE f: INTEGER;
  VAR x: INTEGER;
  BEGIN
    x := 4;
    return x;
  END f;

BEGIN 
    x := 1;
    f;
    return x;
END shadow.

(*
CHECK: 1
*)
