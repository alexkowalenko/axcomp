(*
RUN: %comp %s | echo $? | filecheck %s
CHECK: 0
*)

MODULE Simple;
IMPORT AnotherModule;
END Simple.
