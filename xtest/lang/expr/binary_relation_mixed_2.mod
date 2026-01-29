(*
RUN: %comp %s | filecheck %s
*)

<* MAIN+ *>

module binary_relation_mixed_2;

begin
     WriteBoolean(  1.0 = 2); WriteLn;
     WriteBoolean(  1.0 # 2); WriteLn;
     WriteBoolean(  1.0 > 2); WriteLn;
     WriteBoolean(  1.0 >= 2); WriteLn;
     WriteBoolean(  1.0 < 2); WriteLn;
     WriteBoolean(  1.0 <= 2); WriteLn;
end binary_relation_mixed_2.

(*
CHECK: 0
CHECK-NEXT: 1
CHECK-NEXT: 0
CHECK-NEXT: 0
CHECK-NEXT: 1
CHECK-NEXT: 1
*)
