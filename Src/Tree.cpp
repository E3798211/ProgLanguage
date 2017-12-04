#include "../Head/Tree.h"

/*
 *
 *  =============================================   NODE CLASS  =============================================
 *
 */


/*static*/ bool
Node::IsLast(const Node* check_node)
{
    EnterFunction();

    SAFE {
    // Verificator
    }

    if (check_node->_left == nullptr && check_node->_right == nullptr){
        SetColor(GREEN);
        DEBUG printf("Given pointer is last\n");
        SetColor(DEFAULT);

        QuitFunction();
        return true;
    }else{
        SetColor(GREEN);
        DEBUG printf("Given pointer is not last\n");
        SetColor(DEFAULT);

        QuitFunction();
        return false;
    }
}

/*static*/ Node*
Node::CreateNode(Node* parent_node)
{
    EnterFunction();

    PrintVar(parent_node);

    Node* new_node = nullptr;
    try
    {
        new_node = new Node;
    }
    catch(const std::bad_alloc &ex)
    {
        SetColor(RED);
        DEBUG printf("=====   Cannot create new node   =====\n");
        SetColor(DEFAULT);

        new_node = nullptr;

        PrintVar(new_node);

        QuitFunction();
        return nullptr;
    }

    new_node->_parent   = parent_node;

    PrintVar(parent_node);

    SetColor(GREEN);
    DEBUG printf("New node created\n");
    SetColor(DEFAULT);

    QuitFunction();
    return new_node;
}

// =================================================    SETTERS

int
Node::SetData(double new_data)
{
    _data = new_data;
    return OK;
}

int
Node::SetLeft(Node* left)
{
    _left = left;
    return OK;
}

int
Node::SetRight(Node* right)
{
    _right = right;
    return OK;
}

int
Node::SetParent(Node* parent)
{
    _parent = parent;
    return OK;
}

int
Node::SetDataType(int data_type)
{
    _data_type = data_type;
    return OK;
}

// =================================================    GETTERS

double
Node::GetData()
{
    return _data;
}

Node*
Node::GetLeft()
{
    return _left;
}

Node*
Node::GetRight()
{
    return _right;
}

Node*
Node::GetParent()
{
    return _parent;
}

int
Node::GetDataType()
{
    return _data_type;
}

Node*
Node::Copy(Node* node_to_copy)
{
    EnterFunction();

    Node* new_node = Node::CreateNode();
    if(node_to_copy != nullptr){
        new_node->SetData    (node_to_copy->GetData());
        new_node->SetDataType(node_to_copy->GetDataType());
        new_node->SetLeft    (Node::Copy(node_to_copy->GetLeft()));
        new_node->SetRight   (Node::Copy(node_to_copy->GetRight()));
        new_node->SetParent  (node_to_copy->GetParent());
    }else{
        delete new_node;
        new_node = nullptr;
    }

    QuitFunction();
    return new_node;
}

/*static*/ int
Node::DeleteNode(Node* node_to_delete)
{
    EnterFunction();

    if(node_to_delete == nullptr){
        QuitFunction();
        return OK;
    }

    if(node_to_delete->GetLeft()  != nullptr)    DeleteNode(node_to_delete->GetLeft());
    if(node_to_delete->GetRight() != nullptr)    DeleteNode(node_to_delete->GetRight());

    delete node_to_delete;

    QuitFunction();
    return OK;
}

/*
 *
 *  =============================================   TREE CLASS  =============================================
 *
 */

// =================================================    Supporting functions

// Related to Dot
int
SetNodes(FILE* output, Node* branch_root)
{
    //PrintVar(branch_root);
    if(branch_root == nullptr)      return OK;

    if(output == nullptr){
        SetColor(RED);
        DEBUG printf("=====   Output file for DOT was not opened.   =====\n");
        SetColor(DEFAULT);

        return FILE_NOT_OPENED;
    }

    fprintf(output, "_%p", branch_root);

    fprintf(output, BEGIN_DECLARATION);
    fprintf(output, LABELS);
    fprintf(output, NEXT_FIELD);

    fprintf(output, BEGIN_COLUMN);

    fprintf(output, "_%p",  branch_root);
    fprintf(output, NEXT_FIELD);
    fprintf(output, "%lg", branch_root->GetData());
    fprintf(output, NEXT_FIELD);
    fprintf(output, "%d",  branch_root->GetDataType());
    fprintf(output, NEXT_FIELD);
    fprintf(output, "_%p",  branch_root->GetLeft());
    fprintf(output, NEXT_FIELD);
    fprintf(output, "_%p",  branch_root->GetRight());
    fprintf(output, END_COLUMN);

    fprintf(output, END_DECLARATION);

    return OK;
}

// Related to Dot
int
BuildConnections(FILE* output, Node* branch_root)
{
    if(branch_root == nullptr)      return OK;

    if(branch_root->GetLeft() != nullptr){
        fprintf(output, "_%p", branch_root);
        fprintf(output, TO);
        fprintf(output, "_%p", branch_root->GetLeft());
        fprintf(output, LEFT_DIRECTION);

    }
    if(branch_root->GetRight() != nullptr){
        fprintf(output, "_%p", branch_root);
        fprintf(output, TO);
        fprintf(output, "_%p", branch_root->GetRight());
        fprintf(output, RIGHT_DIRECTION);
    }

    return OK;
}

// =================================================    Private

bool
Tree::BelongsToTree(Node* branch_root, const Node* check_ptr)
{
    if(check_ptr == nullptr)                return false;

    if(branch_root == nullptr)              return false;
    if(branch_root == check_ptr)            return true;

    return (BelongsToTree(branch_root->GetLeft(),  check_ptr) ||
            BelongsToTree(branch_root->GetRight(), check_ptr))?    true : false;
}

int
Tree::PrintBranch(FILE* output, Node* branch_root, int (*print_type)(FILE* output, Node* node_to_print))
{
    print_type(output, branch_root);

    if(branch_root->GetLeft() != nullptr)
        PrintBranch(output, branch_root->GetLeft(), print_type);
    if(branch_root->GetRight() != nullptr)
        PrintBranch(output, branch_root->GetRight(), print_type);


    return OK;
}

// =================================================    Public


Tree::Tree()
{
    EnterFunction();

    /*
    try
    {
        _root = Node::CreateNode();
        _n_nodes = 1;
    }
    catch(const std::bad_alloc &ex)
    {
        SetColor(RED);
        DEBUG printf("=====   Cannot create root node   =====\n");
        SetColor(DEFAULT);
    }
    */

    PrintVar(_root);
    PrintVar(_n_nodes);

    QuitFunction();
}

Tree::~Tree()
{
    EnterFunction();
    if(_alive){
        _alive = false;

        DeleteBranch(_root);
    }
    QuitFunction();
}

Node*
Tree::GetRoot()
{
    return _root;
}

int
Tree::GetNNodes()
{
    return _n_nodes;
}

bool
Tree::IsAlive()
{
    return _alive;
}

Node*
Tree::FindNode(Node* branch_root, const double node_data)
{
    EnterFunction();

    SAFE {
    if(branch_root == nullptr){
        QuitFunction();
        return nullptr;
    }
    }

    if(branch_root->GetData() == node_data){
        QuitFunction();
        return branch_root;
    }

    Node* check_node = nullptr;
    if(branch_root->GetLeft() != nullptr)
        check_node = FindNode(branch_root->GetLeft(), node_data);

    // If found
    if(check_node != nullptr){
        SetColor(GREEN);
        DEBUG printf("Element \"%lg\" found. p = %p\n", check_node->GetData(), check_node);
        SetColor(DEFAULT);

        QuitFunction();
        return check_node;
    }

    if(branch_root->GetRight() != nullptr)
        check_node = FindNode(branch_root->GetRight(), node_data);

    QuitFunction();
    return check_node;
}

int
Tree::SetData(Node* change_node, const double data, int data_type)
{
    EnterFunction();

    PrintVar(change_node);
    PrintVar(data);

    assert(change_node != nullptr);

    SAFE {

    // Verificator

    if(!BelongsToTree(_root, change_node)){
        SetColor(BLUE);
        USR_INFORM printf("Node does not belong to the tree\n");
        SetColor(DEFAULT);

        PrintVar(change_node);
        PrintVar(data);

        QuitFunction();
        return NODE_DOES_NOT_EXIST;
    }
    }

    // =============================

    change_node->SetData(data);
    change_node->SetDataType(data_type);

    // =============================

    PrintVar(change_node);
    PrintVar(data);

    QuitFunction();
    return OK;
}

int
Tree::DeleteBranch(Node* branch_root, int rec_depth, bool right)
{
    assert(branch_root != nullptr);

    SetColor(MAGENTA);
    DEBUG printf("\tIn:   recursion depth = %d, right = %s\n", rec_depth, right? "true" : "false");
    SetColor(DEFAULT);

    PrintVar(branch_root);
    PrintVar(branch_root->GetLeft());
    PrintVar(branch_root->GetRight());

    SAFE {
    if(!BelongsToTree(_root, branch_root)){
        SetColor(BLUE);
        USR_INFORM printf("Node does not belong to the tree\n");
        SetColor(DEFAULT);

        return NODE_DOES_NOT_EXIST;
    }
    }

    if(branch_root->GetLeft() != nullptr)
        DeleteBranch(branch_root->GetLeft(),  rec_depth + 1, false);
    if(branch_root->GetRight() != nullptr)
        DeleteBranch(branch_root->GetRight(), rec_depth + 1, true);

    delete branch_root;
    _n_nodes--;

    SetColor(MAGENTA);
    DEBUG printf("\tQuit: recursion depth = %d, right = %s\n", rec_depth, right? "true" : "false");
    SetColor(DEFAULT);
    return OK;
}

int
Tree::CallGraph()
{
    FILE* output = fopen(DOT_FILENAME_DEFAULT, "w");
    if(output == nullptr){
        SetColor(RED);
        DEBUG printf("=====   Output file for DOT was not opened.   =====\n");
        SetColor(DEFAULT);

        return FILE_NOT_OPENED;
    }

    fprintf(output, BEGIN);
    fprintf(output, SET_RECORD_SHAPE);
    PrintBranch(output, _root, SetNodes);
    PrintBranch(output, _root, BuildConnections);
    fprintf(output, END);

    fclose(output);

    char command[1000];
    strcpy(command, DOT1);
    strcat(command, DOT_FILENAME_DEFAULT);
    strcat(command, DOT2);
    strcat(command, IMG_FILENAME_DEFAULT);

    system(command);

    strcpy(command, OPEN);
    strcat(command, IMG_FILENAME_DEFAULT);

    return system(command);
}

