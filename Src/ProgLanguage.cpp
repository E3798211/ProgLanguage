#include "../Head/ProgLanguage.h"

// =========================================    SUPPORTING FUNCTIONS

/// Returns amount of chars read
/**
    \param [in]  filename               Name of the file to be read
    \param [out] dest                   Buffer to be filled

    \warning Returns -1 in case of error
*/
int FileRead(const char* file_name, char*& dest)
{
    EnterFunction();

    assert(file_name != nullptr);

    FILE* input = fopen(file_name, "r");
    if(input == nullptr){
        SetColor(RED);
        DEBUG printf("=====   File \"%s\" not found!   =====\n", file_name);
        SetColor(DEFAULT);

        QuitFunction();
        return -1;
    }

    if(fseek(input, 0, SEEK_END)){
        SetColor(RED);
        DEBUG printf("=====   Can not set pointer's position to the end   =====\n");
        SetColor(DEFAULT);

        QuitFunction();
        return -1;
    }

    int file_size = ftell(input);
    if(file_size == EOF){
        SetColor(RED);
        DEBUG printf("=====   Can not get pointer's position   =====\n");
        SetColor(DEFAULT);

        QuitFunction();
        return -1;
    }

    //dest = nullptr;
    try
    {
        dest = new char [file_size];
    }
    catch(const std::bad_alloc& ex){
        SetColor(RED);
        DEBUG printf("=====   Can not allocate %d bytes   =====\n", file_size);
        SetColor(DEFAULT);

        QuitFunction();
        return -1;
    }

    rewind(input);
    int n_chars = fread(dest, 1, file_size, input);
    dest[n_chars - 1] = '\0';

    fclose(input);

    QuitFunction();
    return n_chars;
}

/// Checks if given character is space
/**
    Returns true if character is space, false otherwise

    \param [in]  check                  Character to be checked
*/
bool IsSpace(int check)
{
    if(check == ' ')    return true;
    if(check == '\t')   return true;

    return false;
}

/// Deletes spaces from string with programm
/**
    Returns shortened version of programm without spaces or nullptr in case of mistake

    \param [in]  programm               String with original programm
    \param [in]  n_chars                Amount of characters in programm
*/
char* DeleteSpaces(const char* programm, size_t n_chars)
{
    EnterFunction();

    int no_spaces_len = 0;

    int i = 0;
    while(i < n_chars){
        if(!IsSpace(programm[i]))
            no_spaces_len++;
        i++;
    }

    char* no_spaces_programm = nullptr;
    try
    {
        no_spaces_programm = new char [no_spaces_len];
    }
    catch(std::bad_alloc& ex)
    {
        SetColor(RED);
        DEBUG printf("=====   Can not allocate %d bytes   =====\n", no_spaces_len);
        SetColor(DEFAULT);

        QuitFunction();
        return nullptr;
    }

    int k = 0;
        i = 0;
    while(i < n_chars){
        if(!IsSpace(programm[i]))
            no_spaces_programm[k++] = programm[i];
        i++;
    }

    QuitFunction();
    return no_spaces_programm;
}

// =========================================    Recursive descent parser

const char* original = nullptr;
const char* s        = nullptr;
int         p = 0;
bool    error = 0;

int CountLine()
{
    EnterFunction();

    int line_num = 0;
    int i = 0;
    while(i != p)
    {
        if(original[i++] == '\n')    line_num++;
    }

    QuitFunction();
    return line_num;
}

Node*
GetN()
{
    EnterFunction();

    double value = 0;
    int   n_read = 0;

    int dots_met = 0;
    int digits_after_dot = 1;
    while(('0' <= s[p] && s[p] <= '9') || s[p] == '.')
    {
        if       (dots_met == 0)
        {
            value = value * 10 + (s[p] - '0');
        }
        else if (dots_met == 1)
        {
            double multiply = 1;
            for(int i = 0; i < digits_after_dot; i++)
                multiply /= 10;

            value += (s[p] - '0') * multiply;

            digits_after_dot++;
        }
        else
        {
            error = true;

            SetColor(RED);
            printf("Two dots in number. Line: %d\n", CountLine());
            SetColor(DEFAULT);

            QuitFunction();
            return nullptr;
        }

        n_read++;
        p++;
    }

    if(n_read <= 0){
        error = true;

        SetColor(RED);
        printf("Nothing read in GetN()\n", CountLine());
        SetColor(DEFAULT);

        QuitFunction();
        return nullptr;
    }

    Node* new_node = Node::CreateNode();
    new_node->SetData(value);
    new_node->SetDataType(CONSTANT);

    PrintVar(new_node->GetData());

    QuitFunction();
    return new_node;
}

Node*
GetP()
{
    EnterFunction();

    if(s[p] == '(')
    {
        p++;
        Node* new_node = GetE();

        assert(s[p] == ')');
        p++;

        QuitFunction();
        return new_node;
    }
    else
    {
        QuitFunction();
        return GetN();
    }
}

Node*
GetT()
{
    EnterFunction();

    Node* first_factor  = GetP();
    if(error)           return 0;

    Node* top_operation = nullptr;
    Node* current       = nullptr;

    int times_in_loop = 0;
    while(s[p] == '*' || s[p] == '/')
    {
        int op = s[p];
        p++;

        Node* second_factor = GetP();

        // Creating tree
        // We contain the most right node in \current. If we see new operation we create a copy of \current (\tmp), change
        // current into a BIN_OPERATION with \tmp as left branch and new operand as right. So our tree grows to the right

        times_in_loop++;
        if      (times_in_loop == 1){

            top_operation = Node::CreateNode();
            top_operation->SetLeft (first_factor);
            top_operation->SetRight(second_factor);
            top_operation->SetDataType(BIN_OPERATION);

            if(op == '*')               top_operation->SetData('*');
            else                        top_operation->SetData('\\');

            current = top_operation->GetRight();

        }else{

            current->SetLeft (Node::Copy(current));
            current->SetRight(second_factor);
            current->SetDataType(BIN_OPERATION);

            if(op == '*')               current->SetData('*');
            else                        current->SetData('\\');

            current = current->GetRight();
        }

    }

    if(times_in_loop < 1){
        QuitFunction();
        return first_factor;
    }

    QuitFunction();
    return top_operation;
}

Node*
GetE()
{
    EnterFunction();

    Node* first_term = GetT();
    if(error)           return nullptr;

    Node* top_operation = nullptr;
    Node* current       = nullptr;

    int times_in_loop = 0;
    while(s[p] == '+' || s[p] == '-')
    {
        int op = s[p];
        p++;

        Node* second_term = GetT();

        // Creating tree
        // We contain the most right node in \current. If we see new operation we create a copy of \current (\tmp), change
        // current into a BIN_OPERATION with \tmp as left branch and new operand as right. So our tree grows to the right

        times_in_loop++;
        if      (times_in_loop == 1){

            top_operation = Node::CreateNode();
            top_operation->SetLeft (first_term);
            top_operation->SetRight(second_term);
            top_operation->SetDataType(BIN_OPERATION);

            if(op == '+')               top_operation->SetData('+');
            else                        top_operation->SetData('-');

            current = top_operation->GetRight();

        }else{

            current->SetLeft (Node::Copy(current));
            current->SetRight(second_term);
            current->SetDataType(BIN_OPERATION);

            if(op == '*')               current->SetData('+');
            else                        current->SetData('-');

            current = current->GetRight();
        }
    }

    if(times_in_loop < 1){
        QuitFunction();
        return first_term;
    }

    QuitFunction();
    return top_operation;
}

Node*
GetGO(const char* original_expr, const char* expr)
{
    original = original_expr;
    s = expr;
    p = 0;

    Node* top_operand = GetE();
    if(s[p] != '\0'){
        SetColor(RED);
        DEBUG printf("Error in line %d\n", CountLine());
        SetColor(DEFAULT);
    }

    if(error)           return nullptr;
    return top_operand;
}

// =========================================    BRAIN

int
BuildSyntaxTree(Tree* tree)
{
    EnterFunction();

    char* programm = nullptr;
    int n_chars    = FileRead(USR_CODE, programm);
    if(programm == nullptr){
        SetColor(RED);
        DEBUG printf("=====   programm == nullptr   =====\n");
        SetColor(DEFAULT);

        QuitFunction();
        return FILE_NOT_OPENED;
    }

    char* no_spaces_programm = DeleteSpaces(programm, n_chars);

    //delete tree->GetRoot();
    tree->_root = GetGO(programm, no_spaces_programm);

    QuitFunction();
    return OK;
}

int
CompileCode()
{
    Tree tree;

    BuildSyntaxTree(&tree);
    // TranslateCode(tree);
}



