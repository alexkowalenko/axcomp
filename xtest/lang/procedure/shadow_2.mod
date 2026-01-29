(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

MODULE shadow_2;

const x = 1;

PROCEDURE f: INTEGER;
  const x = 2;
  BEGIN
    return x;
  END f;

BEGIN 
    WriteInt(f()); WriteLn;
    return x;
END shadow_2.


(*
CHECK: 2
CHECK-NEXT: 1
*)
