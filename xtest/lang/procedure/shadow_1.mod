(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

MODULE shadow_1;

const x = 1;

PROCEDURE f: INTEGER;
  const x = 2;
  BEGIN
    return x;
  END f;

BEGIN 
    f;
    return x;
END shadow_1.

(*
CHECK: 1
*)
