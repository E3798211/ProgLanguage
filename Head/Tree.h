#ifndef TREE_H_INCLUDED
#define TREE_H_INCLUDED

#include <stdio.h>
#include <assert.h>
#include <cstring>
#include "../Head/Errors.h"
#include "../Head/DotSyntax.h"

// =================================================

// =========================    Safe mode.
//
//  In safe mode programm checks if node exists, runs verificator, etc.
//  In case you are sure that your programm will work correct and you need to
//  speed it up, turn safe-mode off.
//

// Comment next line to turn safe-mode off.
//#define _SAFE
#ifdef _SAFE
    #define SAFE
#else
    #define SAFE if(0)
#endif // _USR_INFORM

// =========================    Print information for user
#define _USR_INFORM
#ifdef _USR_INFORM
    #define USR_INFORM
#else
    #define USR_INFORM if(0)
#endif // _USR_INFORM

// =========================    Print information debug information
// Comment next line to turn debug off.

#define _DEBUG
#include "DebugLib.h"

// =================================================

enum NodeTypes {
    GLOBAL_NODE,            // 0
    DECLARE_FUNCTION,       // 1
    CALL_FUNCTION,          // 2
    MAIN_FUNCTION,          // 3

    DECLARE_ARGUMENT_LIST,  // 4
    CALL_ARGUMENT_LIST,     // 5
    ARGUMENT,               // 6
    RETURN,                 // 7

    VARIABLE_TO_CREATE,     // 8

    CONSTANT,               // 9
    VARIABLE,               // 10

    BIN_OPERATION,          // 11
    OPERATOR,               // 12
    ASM_INSERT,             // 13
    ASM_CODE_BEG            // 14
};

enum Operators {
    IF_OP,
    UNTIL_OP,
    ASSIGN_OP,
    COMPOSITE_OP
};

// =================================================

class Node {
private:
    /// String with data
    double   _data = 0;

    /// Pointer to the left node
    Node*    _left = nullptr;

    /// Pointer to the right node
    Node*   _right = nullptr;

    /// Pointer to the parent node
    Node*  _parent = nullptr;

    /// Varriable responsible of how to understand data
    int _data_type = 0;

public:
    /**
        Returns nullptr if some errors occured

        \param [in] parent_node     Pointer to the parent
    */

    static Node* CreateNode(Node* parent_node = nullptr);

    /// Check if node is last or not
    /**
        Rerturns true if elem is last, false othewise.

        \param [in] check_node      Pointer to the node you want to check
    */
    static bool  IsLast(const Node* check_node);
    static Node* Copy(Node* node_to_copy);
    static int   DeleteNode(Node* node_to_delete);

    int SetData    (double new_data);
    int SetLeft    (Node* left);
    int SetRight   (Node* right);
    int SetParent  (Node* parent);
    int SetDataType(int data_type);

    double  GetData();
    Node*   GetLeft();
    Node*  GetRight();
    Node* GetParent();
    int GetDataType();
};

class Tree {
private:
public:
    /// Pointer to the first node
    Node* _root  = nullptr;

    /// Nodes counter
    int _n_nodes = 0;

    /// Alive or deleted
    bool _alive = true;

// =========================    Supporting functions
    /// Prints nodes for DOT
    /**
        Function provides with go-round of the branch

        \param [in] output          FILE to write in
        \param [in] branch_root     Pointer to the root of the branch
        \param [in] print_type      Pointer to the function that prints nodes
    */
    int PrintBranch(FILE* output, Node* branch_root, int (*print_type)(FILE* output, Node* node_to_print));

public:
    /// Default constructor
    Tree();

    /// Destructor
    ~Tree();

    /// Checks node existence
    /**
        Retutns TRUE if node exists

        \param [in] branch_root     Pointer to te first element in branch we check
        \param [in] node            Pointer we want to check
    */
    bool BelongsToTree(Node* branch_root, const Node* check_ptr);

    /// Gets root
    Node* GetRoot();

    /// Gets n_nodes
    int GetNNodes();

    /// Finds node in the tree
    /**
        Returns Pointer to the node if found, nullptr otherwise
        \param [in] branch_root     Pointer to the root of the branch
        \param [in] node_data       Data we are looking for
    */
    Node* FindNode(Node* branch_root, const double node_data);

    /// Gets status - dead or alive
    bool IsAlive();

    /// Set data
    /**
        Returns error code

        \param [in]  change_node    Ponter to the node you want to change
        \param [out] data           New data
        \param [in]  data_type      Type of the data
    */
    int SetData(Node* change_node, const double data, int data_type);

    /// Delete branch
    /**
        \param [in] branch_root     Pointer to the root node of the branch we want to delete
        \param [in] dbg_rec_depth   DEBUG VARRIABLE, PLEASE DO NOT USE IT
        \param [in] dbg_right       DEBUG VARRIABLE, PLEASE DO NOT USE IT

        \warning Deletes branch RECURSIVELY! Use only first parameter or you will not see correct debug info.
        Deletes root too.

        \warning Does not change parent's pointer!
    */
    int DeleteBranch(Node* branch_root, int dbg_rec_depth = 0, bool dbg_right = true);

    /// Calls grapic dump
    int CallGraph();

};


#endif // TREE_H_INCLUDED
