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

// =========================================    Recursive descent parser

/*
 *
 *  Key-words
 *
 */

#define VAR     "var"
#define FUNK    "def"
#define IF      "if"
#define LOOP    "until"

#define MAX_VAR_AMNT        100
#define MAX_VAR_NAME_LEN    100

char variables[MAX_VAR_AMNT][MAX_VAR_NAME_LEN] = {};
int  n_variables = 0;

const char* s = nullptr;
int         p = 0;
bool    error = 0;

void
SkipSpaces()
{
    while(s[p] == ' ' || s[p] == '\t' || s[p] == '\v')      p++;
}

char* GetWord(char* word)
{
    EnterFunction();

    SkipSpaces();
    //char word[MAX_VAR_NAME_LEN] = {'\0'};
    int  word_len = 0;
    while(('a' <= s[p] && s[p] <= 'z') ||
          ('A' <= s[p] && s[p] <= 'Z') || s[p] == '_')
    {
        if(word_len < MAX_VAR_NAME_LEN)
            word[word_len++] = s[p];
        p++;
    }

    if(word_len == 0){
        QuitFunction();
        return nullptr;
    }

    word[word_len] = '\0';

    QuitFunction();
    return word;
}

//                        Создание переменной - всегда выдача последнего свободного места в оперативе

int
CountLine()
{
    EnterFunction();

    int line_num = 0;
    int i = 0;
    while(i != p)
    {
        if(s[i++] == '\n')    line_num++;
    }

    QuitFunction();
    return line_num;
}

Node*
GetN()
{
    EnterFunction();

    double value = 0;
    char* num_end = nullptr;

    SkipSpaces();
    value = strtod(s + p, &num_end);

    if(num_end == s + p){
        error = true;

        SetColor(RED);
        printf("Nothing read in GetN()\n", CountLine());
        SetColor(DEFAULT);

        QuitFunction();
        return nullptr;
    }

    p += num_end - s - p;

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

    SkipSpaces();
    if(s[p] == '(')
    {
        p++;
        Node* new_node = GetE();

        SkipSpaces();
        assert(s[p] == ')');
        p++;

        QuitFunction();
        return new_node;
    }
    else if(('a' <= s[p] && s[p] <= 'z') ||
            ('A' <= s[p] && s[p] <= 'Z') || s[p] == '_')
    {
        char word[MAX_VAR_NAME_LEN] = {};
        GetWord(word);

        Node* new_node = Node::CreateNode();
        new_node->SetDataType(VARIABLE);

        int i = 0;
        bool var_exists = false;
        while(i < n_variables){
            if(!strcmp(word, variables[i])){
                new_node->SetData(i);

                var_exists = true;
            }
            i++;
        }

        if(!var_exists){
            strcpy(variables[n_variables], word);
            new_node->SetData(n_variables);

            n_variables++;
        }
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
    SkipSpaces();
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
            /*
            current->SetLeft (Node::Copy(current));
            current->SetRight(second_factor);
            current->SetDataType(BIN_OPERATION);

            if(op == '*')               current->SetData('*');
            else                        current->SetData('\\');

            current = current->GetRight();
            */

            current->SetLeft (Node::Copy(current));
            current->SetRight(second_factor);
            current->SetDataType(BIN_OPERATION);

            if(op == '*')               current->SetData('*');
            else                        current->SetData('\\');

            current = current->GetRight();
        }

        SkipSpaces();
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

    // ==
    // >
    // <

    SkipSpaces();
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

            if(op == '+')               current->SetData('+');
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

Node* GetOperator()
{
    EnterFunction();

    // var
    // =
    // if
    // until
    // { ... }

    QuitFunction();
}

Node*
GetGO(const char* expr)
{
    s = expr;
    p = 0;

    // GetVarNames

    p = 0;
    Node* top_operand = GetE();
    if(s[p] != '\n' && s[p] != '\0'){
        int error_pos = CountLine();
        SetColor(RED);
        DEBUG printf("G0: Error in line %d\n", error_pos);
        SetColor(DEFAULT);
        PrintVar(p);
        PrintVar(s[p]);
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
    FileRead(USR_CODE, programm);
    if(programm == nullptr){
        SetColor(RED);
        DEBUG printf("=====   programm == nullptr   =====\n");
        SetColor(DEFAULT);

        QuitFunction();
        return FILE_NOT_OPENED;
    }

    tree->_root = GetGO(programm);

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



