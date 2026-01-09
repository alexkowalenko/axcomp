(*
RUN: %comp %s | filecheck %s
CHECK: 0
*)

MODULE str1;
BEGIN
    RETURN 0; (* Address of the following string changes *)
    RETURN "hello";
END str1.