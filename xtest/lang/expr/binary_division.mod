(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

module binary_division;

import Out;

begin
    Out.Real(7.0 / 2, 3); Out.Ln;
end binary_division.

(*
CHECK: 3.5
*)
