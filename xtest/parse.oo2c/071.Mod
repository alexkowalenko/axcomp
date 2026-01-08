(*
RUN: %comp %s | echo $? | filecheck %s
CHECK: 0
*)

MODULE M;
BEGIN
  CASE x OF
  END
END M.
