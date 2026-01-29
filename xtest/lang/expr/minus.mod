(*
RUN: %comp %s | filecheck %s
*)

module minus;
begin
    return -10;
end minus.

(* -10
CHECK: 246
*)
