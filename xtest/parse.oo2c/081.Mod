(*
RUN: %comp %s | echo $? | filecheck %s
CHECK: 0
*)

MODULE M;
BEGIN
  REPEAT
    x := 1
  UNTIL a
END M.
