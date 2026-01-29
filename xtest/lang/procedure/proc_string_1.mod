(*
RUN: %comp %s | filecheck %s
CHECK: Hello
*)

<* MAIN+ *>

MODULE proc_string_1;
IMPORT Out;

VAR
    c : STRING;

PROCEDURE f(s: STRING);
BEGIN
    Out.String(s); Out.Ln;
END f;

BEGIN
    c := "Hello";
    f(c);
END proc_string_1.
