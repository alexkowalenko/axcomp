MODULE a1;
BEGIN
    RETURN 12
END a1.

(*
RUN: %comp %s | filecheck %s
CHECK: 12
*)
