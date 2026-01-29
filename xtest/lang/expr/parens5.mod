(*
RUN: %comp %s | filecheck %s
*)

module parens5;
begin
    return -((((17))));
end parens5.

(* -17
CHECK: 239
*)
