(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)

MODULE b12;
BEGIN
    RETURN (1 < 2) = FALSE;
    RETURN 1 <= 2;
    RETURN 1 > 2;
    RETURN 1 >= 2
END b12.