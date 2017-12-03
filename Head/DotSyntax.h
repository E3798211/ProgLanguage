#ifndef DOTSYNTAX_H_INCLUDED
#define DOTSYNTAX_H_INCLUDED

const char DOT_FILENAME_DEFAULT[] = "dotdef.dot";
const char IMG_FILENAME_DEFAULT[] = "dotdef.ps";

const char OPEN[] = "xdg-open ";

const char DOT1[] = "dot -Tps ";
const char DOT2[] = " -o ";


const char             BEGIN[]  = "digraph G {\n";
const char               END[]  = "}\n";

const char  SET_RECORD_SHAPE[]  = "node[shape = record];\n";

const char BEGIN_DECLARATION[]  = " [label = \"";
const char   END_DECLARATION[]  = "\"];\n";
const char      BEGIN_COLUMN[]  = "{ ";
const char        END_COLUMN[]  = " }";
const char        NEXT_FIELD[]  = " | ";
const char            LABELS[]  = "{Pointer | Data | Data type | Left | Right } ";


const char   RIGHT_DIRECTION[]  = " [color = green];\n";
const char    LEFT_DIRECTION[]  = " [color = blue];\n";
const char   WRONG_DIRECTION[]  = " [color = red];\n";

const char                TO[]  = " -> ";

#endif // DOTSYNTAX_H_INCLUDED
