MODULE a4; (* hello *)
BEGIN
    RETURN FALSE (* should be 0 *)
END a4.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)
