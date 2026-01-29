MODULE loop_3;
BEGIN
    EXIT;
    RETURN 0;
END loop_3.

(*
RUN:  %comp %s | filecheck %s
CHECK: [3,8]: EXIT: no enclosing loop.
*)

