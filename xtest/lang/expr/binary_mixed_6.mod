(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

module binary_mixed_6;

import Out;

begin
     Out.Real(  2.4 / 2, 2); Out.Ln;
end binary_mixed_6.

(*
CHECK: 1.2
*)
