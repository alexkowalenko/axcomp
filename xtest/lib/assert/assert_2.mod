MODULE assert_2; (* ASSERT *)
VAR x: INTEGER;
BEGIN
    ASSERT(x = 0, 3);
    ASSERT(x # 0, 2);
END assert_2.

(*
RUN: %comp %s | filecheck %s
CHECK: 2
*)
