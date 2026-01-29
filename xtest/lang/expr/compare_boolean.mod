
(*
RUN: %comp %s | filecheck %s
*)

MODULE compare_boolean;
BEGIN
    WriteBoolean(FALSE = TRUE); WriteLn;
    WriteBoolean(FALSE # TRUE); WriteLn;
    WriteBoolean(TRUE = TRUE); WriteLn;
    WriteBoolean(TRUE # TRUE); WriteLn;

    WriteBoolean(FALSE = FALSE); WriteLn;
    WriteBoolean(FALSE # FALSE); WriteLn;
    WriteBoolean(TRUE = FALSE); WriteLn;
    WriteBoolean(TRUE # FALSE); WriteLn;
END compare_boolean.

(*
CHECK: 0
CHECK-NEXT: 1
CHECK-NEXT: 1
CHECK-NEXT: 0
CHECK-NEXT: 1
CHECK-NEXT: 0
CHECK-NEXT: 0
CHECK-NEXT: 1
*)
