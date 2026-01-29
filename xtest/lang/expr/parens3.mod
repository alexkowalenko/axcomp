(*
RUN: %comp %s | filecheck %s
*)

module parens3;
begin
    return ((-(17)));
end parens3.

(* -17
CHECK: 239
*)
