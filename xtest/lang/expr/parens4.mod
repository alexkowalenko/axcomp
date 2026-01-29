(*
RUN: %comp %s | filecheck %s
*)

module parens4;
begin
    return -(+(-(17)));
end parens4.

(*
CHECK: 17
*)
