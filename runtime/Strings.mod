MODULE Strings;

PROCEDURE Length* (s: STRING): INTEGER;
END Length;

PROCEDURE Concat* (s1, s2: STRING): STRING;
END Concat;

PROCEDURE ConcatChar* (s: STRING; c: CHAR): STRING;
END ConcatChar;

PROCEDURE AppendChar* (c: CHAR; s: STRING): STRING;
END AppendChar;

PROCEDURE Compare* (s1, s2: STRING): INTEGER;
END Compare;

END Strings.