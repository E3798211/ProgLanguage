#ifndef PROGLANGUAGE_H_INCLUDED
#define PROGLANGUAGE_H_INCLUDED

#include "Tree.h"

// =========================================    DEFAULT FILES

const char USR_CODE[] = "main.txt";
const char ASM_CODE[] = "main.txt";

// =========================================    Recursive descent parser

Node* GetN();
Node* GetP();
Node* GetT();
Node* GetE();
Node* GetGO(const char* expr);

// =========================================    BRAIN

int BuildSyntaxTree(Tree* tree);

int CompileCode();

#endif // PROGLANGUAGE_H_INCLUDED
