file: /usr/local/epics/base-3.14.9_tornado221/src/dbtools/dbLoadTemplate_lex.l

line with bareword definition should have \$ in order 
to allow $ character to be passed to PMAC database from dbLoadTemplate() call:

bareword    [a-zA-Z0-9_\-+:./\\\[\]<>;\$]
