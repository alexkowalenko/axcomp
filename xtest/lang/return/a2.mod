MODULE a2;
BEGIN
    RETURN 12;
    RETURN 24
END a2.

(*
RUN: %comp %s | filecheck %s
CHECK: 12
*)
