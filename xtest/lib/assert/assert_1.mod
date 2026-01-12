MODULE assert_1; (* ASSERT *)
BEGIN
    ASSERT(TRUE);
END assert_1.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)
