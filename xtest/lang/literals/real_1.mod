(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

module real_1;
import Out;

begin
    Out.Real(3.4,3); Out.Ln;
end real_1.

(*
CHECK: 3.4
*)
