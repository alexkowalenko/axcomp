(*
RUN: %comp %s | echo $? | filecheck %s
CHECK: 0
*)

MODULE M;

CONST
  x = 1;
  (**And this comment is attached to the name of the constant declaration.  *)

END M.
