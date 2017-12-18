#ifndef PROGLANGUAGE_H_INCLUDED
#define PROGLANGUAGE_H_INCLUDED

#include "Tree.h"
#include "Preprocessor.h"
#include <ctype.h>

// =========================================    DEFAULT FILES

const char USR_CODE[] = "main.txt";
const char ASM_CODE[] = "out.txt";

// =========================================    Recursive descent parser

Node* GetN();
Node* GetP();
Node* GetT();
Node* GetE();
Node* GetOperator();
Node* GetFunctions();
Node* GetGO(const char* expr);

// =========================================    BRAIN

/// Builds syntax tree
/**
    \param [out] tree               Tree to be created
*/
int BuildSyntaxTree(Tree* tree);

/// Creates file with asm code
int CompileCode();

/// Gets round the tree and writes correct asm code to output file
/**
    Responsible for Function nodes and higher

    \param [in] output              File where to write
    \param [in] root_node           Pointer to the root of the tree
*/
int TranslateCode_1_lvl(FILE* output, Node* root_node);

/// Gets round branches and writes correct asm code to output file
/**
    Responsible for Operators nodes and lower

    \param [in] output              File where to write
    \param [in] root_node           Pointer to the root of the tree
*/
int TranslateCode_2_lvl(FILE* output, Node* root_node);

#endif // PROGLANGUAGE_H_INCLUDED
