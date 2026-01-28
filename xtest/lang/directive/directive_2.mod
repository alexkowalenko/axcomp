(*
RUN: %ax --run %s | filecheck %s
*)

<* main+ *>

MODULE directive_2;
BEGIN
    WriteBoolean(TRUE); WriteLn;
END directive_2.

(*
CHECK: 1
*)
