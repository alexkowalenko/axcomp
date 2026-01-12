MODULE a3; (* hello *)
BEGIN
    RETURN 12; (* not returned*)
    RETURN 24 (* returned *)
END a3.

(*
RUN: %comp %s | filecheck %s
CHECK: 12
*)
