(*
RUN: %comp %s | filecheck %s
*)

module parens;
begin
    return -(17);
end parens.

(* -17
CHECK: 239
*)
