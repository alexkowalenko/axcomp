(*
RUN: %comp %s | filecheck %s
CHECK: Hello
*)

<* MAIN+ *>

MODULE proc_string_2;
IMPORT Out;

PROCEDURE f():STRING;
BEGIN
    RETURN "Hello";
END f;

BEGIN
    Out.String(f()); Out.Ln; 
END proc_string_2.
