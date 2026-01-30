MODULE comment_slash;
VAR x: INTEGER;
BEGIN
    // line comment before assignment
    x := 1; // trailing comment
    // another comment
    x := x + 1
    RETURN x;
END comment_slash.

//RUN: %comp %s | filecheck %s
//CHECK: 2
