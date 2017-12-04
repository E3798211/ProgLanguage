#ifndef PROGLANGUAGE_H_INCLUDED
#define PROGLANGUAGE_H_INCLUDED

#include "Tree.h"

// =========================================    DEFAULT FILES

const char USR_CODE[] = "main.txt";
const char ASM_CODE[] = "out.txt";

// =========================================    Recursive descent parser

Node* GetN();
Node* GetP();
Node* GetT();
Node* GetE();
Node* GetGO(const char* expr);

// =========================================    BRAIN

/// Builds syntax tree
/**
    \param [out] tree               Tree to be created
*/
int BuildSyntaxTree(Tree* tree);

/// Creates file with asm code
int CompileCode();

#endif // PROGLANGUAGE_H_INCLUDED
