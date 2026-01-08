(*
RUN: %comp %s | echo $? | filecheck %s
CHECK: 0
*)

MODULE Simple;
END Simple.
