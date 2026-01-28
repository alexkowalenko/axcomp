(*
RUN: %ax --run %s | filecheck %s
*)

<* MAIN + *>

MODULE directive;
BEGIN
    WriteBoolean(TRUE); WriteLn;
END directive.

(*
CHECK: 1
*)
