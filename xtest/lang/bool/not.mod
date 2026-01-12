MODULE not;
    VAR x : BOOLEAN;
BEGIN
    WriteBoolean(~TRUE); WriteLn;
    WriteBoolean(~FALSE); WriteLn;
    WriteBoolean(~~TRUE); WriteLn;
    x := TRUE;
    WriteBoolean(x); WriteLn;
    x := ~x;
    WriteBoolean(x); WriteLn;
    RETURN 0;
END not.

(*
RUN: %comp %s | filecheck %s
CHECK: 0
CHECK-NEXT: 1
CHECK-NEXT: 1
CHECK-NEXT: 1
CHECK-NEXT: 0
CHECK-NEXT: 0
*)
