MODULE pointer01; (* pointers *)
VAR x : POINTER TO INTEGER;
BEGIN
    x := NIL;
    RETURN 0;
END pointer01.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)