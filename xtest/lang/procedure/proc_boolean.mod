
(*
RUN: %comp %s | filecheck %s
CHECK: 0
CHECK: 1
*)

<* MAIN+ *>

MODULE proc_boolean;

IMPORT Out;

VAR
    x : BOOLEAN;

PROCEDURE Write(x: BOOLEAN);
BEGIN
    Out.Bool(x); Out.Ln;
END Write;

BEGIN
    x := FALSE;
    Write(x);
    x := TRUE;
    Write(x);
END proc_boolean.

