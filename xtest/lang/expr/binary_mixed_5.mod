(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

module binary_mixed_5;

import Out;

begin
    Out.Real(  2.4 * 2, 3); Out.Ln;
end binary_mixed_5.

(*
CHECK: 4.8
*)
