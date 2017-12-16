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

#define FUNC    "function"
#define MAIN    "main"
#define IF      "if"
#define UNTIL   "until"
#define VAR     "var"


#define MAX_VAR_AMNT        100
#define MAX_VAR_NAME_LEN    100
char variables[MAX_VAR_AMNT][MAX_VAR_NAME_LEN] = {};
int  n_variables = 0;

#define MAX_FUNC_AMNT       100
#define MAX_FUNC_NAME_LEN   100
char functions[MAX_FUNC_AMNT][MAX_FUNC_NAME_LEN] = {};
int n_functions = 0;

const char* s = nullptr;
int         p = 0;
int     s_len = 0;
bool    error = 0;

void SkipSpaces()
{
    while(s[p] == ' ' || s[p] == '\t' || s[p] == '\v')      p++;
}
void SkipEnters()
{
    while(s[p] == '\n')     p++;
}
void SkipAllSpaces()
{
    while(isspace(s[p]))    p++;
}
int GetWord(char* word)
{
    EnterFunction();

    //SkipSpaces();
    //SkipEnters();
    //SkipSpaces();
    SkipAllSpaces();
    int word_len = 0;
    int position = p;
    while(('a' <= s[position] && s[position] <= 'z') ||
          ('A' <= s[position] && s[position] <= 'Z') || s[position] == '_')
    {
        if(word_len < MAX_VAR_NAME_LEN)
            word[word_len++] = s[position];
        //p++;
        position++;
    }

    if(word_len == 0){
        QuitFunction();
        return 0;
    }

    word[word_len] = '\0';

    QuitFunction();
    return word_len;
}

//                        Создание переменной - всегда выдача последнего свободного места в оперативе?

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

Node* GetN()
{
    if(error)       return nullptr;

    EnterFunction();

    PrintVar(p);
    PrintVar(s[p]);

    double value = 0;
    char* num_end = nullptr;

    SkipSpaces();
    value = strtod(s + p, &num_end);

    if(num_end == s + p){
        error = true;
        PrintVar(error);

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

Node* GetP()
{
    if(error)       return nullptr;

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
        SkipEnters();
        char word[MAX_VAR_NAME_LEN] = {};
        p += GetWord(word);

        int i = 0;
        bool var_exists = false;
        while(i < n_variables){
            if(!strcmp(word, variables[i])){
                var_exists = true;
                break;
            }
            i++;
        }

        if(!var_exists){
            SetColor(RED);
            printf("=====   %s was not declared in this scope   =====\n", word);
            SetColor(DEFAULT);

            error = true;
            PrintVar(error);

            return nullptr;
        }

        Node* new_node = Node::CreateNode();
        new_node->SetDataType(VARIABLE);
        new_node->SetData(i);

        SkipSpaces();

        QuitFunction();
        return new_node;
    }
    else
    {
        QuitFunction();
        return GetN();
    }
}

Node* GetT()
{
    if(error)       return nullptr;

    EnterFunction();

    Node* first_factor  = GetP();

    if(error)       return nullptr;
    Node* top_operation  = nullptr;
    Node* current        = nullptr;

    int times_in_loop = 0;
    SkipSpaces();
    while(s[p] == '*' || s[p] == '/')
    {
        int op = s[p++];

        Node* second_factor = GetP();

        // Creating tree
        // We contain the most right node in \current. If we see new operation we create a copy of \current (\tmp), change
        // current into a BIN_OPERATION with \tmp as left branch and new operand as right. So our tree grows to the right

        times_in_loop++;
        if      (times_in_loop == 1){

            top_operation = Node::CreateNode();
            top_operation->SetLeft      (first_factor);
            top_operation->SetRight     (second_factor);
            top_operation->SetDataType  (BIN_OPERATION);

            if(op == '*')               top_operation->SetData('*');
            else                        top_operation->SetData('\\');

            current = top_operation->GetRight();

        }else{

            current->SetLeft            (Node::Copy(current));
            current->SetRight           (second_factor);
            current->SetDataType        (BIN_OPERATION);

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

Node* GetE()
{
    if(error)           return nullptr;

    EnterFunction();
    Node* first_term = GetT();
    if(error)       return nullptr;

    Node* top_operation  = nullptr;
    Node* current        = nullptr;

    int times_in_loop = 0;

    //SkipSpaces();
    //SkipEnters();
    //SkipSpaces();
    SkipAllSpaces();

    while(s[p] == '+' || s[p] == '-' || s[p] == '>' || s[p] == '<' || s[p] == '~')
    {
        int op = s[p++];

        Node* second_term = GetT();

        // Creating tree
        // We contain the most right node in \current. If we see new operation we create a copy of \current (\tmp), change
        // current into a BIN_OPERATION with \tmp as left branch and new operand as right. So our tree grows to the right

        times_in_loop++;
        if      (times_in_loop == 1){

            top_operation = Node::CreateNode();
            top_operation->SetLeft      (first_term);
            top_operation->SetRight     (second_term);
            top_operation->SetDataType  (BIN_OPERATION);

            switch(op)
            {
                case '+':
                {
                    top_operation->SetData('+');
                    break;
                }
                case '-':
                {
                    top_operation->SetData('-');
                    break;
                }case '>':
                {
                    top_operation->SetData('>');
                    break;
                }case '<':
                {
                    top_operation->SetData('<');
                    break;
                }
                case '~':
                {
                    top_operation->SetData('~');
                    break;
                }

            }

            current = top_operation->GetRight();

        }else{

            current->SetLeft        (Node::Copy(current));
            current->SetRight       (second_term);
            current->SetDataType    (BIN_OPERATION);

            switch(op)
            {
                case '+':
                {
                    current->SetData('+');
                    break;
                }
                case '-':
                {
                    current->SetData('-');
                    break;
                }case '>':
                {
                    current->SetData('>');
                    break;
                }case '<':
                {
                    current->SetData('<');
                    break;
                }
                case '~':
                {
                    current->SetData('~');
                    break;
                }

            }

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
    if(error)       return nullptr;

    EnterFunction();
    char word[MAX_VAR_NAME_LEN] = {};
    int shift                   = GetWord(word);

    Node* new_node = nullptr;

    if(!strcmp(word, IF) || !strcmp(word, UNTIL)){

        p += shift;

        Node* condition = nullptr;

        // Taking condition from brackets
        SkipSpaces();
        if(s[p] == '(')
        {
            p++;
            condition = GetE();
            SkipSpaces();

            PrintVar(p);
            assert(s[p] == ')');
            p++;
        }else{
            error = true;
            PrintVar(error);

            QuitFunction();
            return nullptr;
        }

        // Taking code to be executed
        Node* code = GetOperator();

        new_node = Node::CreateNode();
        new_node->SetDataType(OPERATOR);

        if(!strcmp(word, IF))           new_node->SetData(IF_OP);
        else                            new_node->SetData(UNTIL_OP);

        new_node->SetLeft (code);
        new_node->SetRight(condition);

        QuitFunction();
        return new_node;

    }
    else if(!strcmp(word, VAR)){

        p += shift + 1;
        p += GetWord(word);

        int i = 0;
        while(i < n_variables){
            if(!strcmp(word, variables[i])){
                SetColor(RED);
                printf("=====   Redeclaration of %s   =====\n", word);
                SetColor(DEFAULT);

                error = true;
                PrintVar(error);

                QuitFunction();
                return nullptr;
            }
            i++;
        }

        strcpy(variables[n_variables], word);
        n_variables++;

        Node* var = Node::CreateNode();
        var->SetDataType(VARIABLE_TO_CREATE);
        var->SetData    (i);

        return var;

    }
    else if(s[p] == '{'){

        p++;

        SkipSpaces();
        if(s[p] != '\n'){
            error = true;
            PrintVar(error);
        }
        p++;

        Node* current = nullptr;

        SkipSpaces();
        int times_in_loop = 0;
        while(/*s[p] != '}'*//*1*/1){

            SkipSpaces();
            SkipEnters();
            if(s[p] == '}'){
                p++;
                break;
            }

            Node* next_expr = GetOperator();
            if(error)       return nullptr;

            times_in_loop++;
            if(times_in_loop == 1){

                new_node = Node::Copy   (next_expr);
                Node::DeleteNode        (next_expr);

            }else if (times_in_loop == 2){

                Node* tmp = Node::Copy  (new_node);
                Node::DeleteNode        (new_node);

                new_node->CreateNode();
                new_node->SetRight      (Node::Copy(tmp));
                new_node->SetDataType   (OPERATOR);
                new_node->SetData       (COMPOSITE_OP);
                new_node->SetLeft       (next_expr);

                Node::DeleteNode        (tmp);

                current = new_node->GetLeft();

            }else{

                //Node* tmp = Node::Copy  (current);

                current->SetRight       (Node::Copy(current));
                current->SetDataType    (OPERATOR);
                current->SetData        (COMPOSITE_OP);
                current->SetLeft        (next_expr);

                current = current->GetLeft();
            }

            if(p > s_len){
                error = true;
                SetColor(RED);
                printf("=====   No legal ending found   =====\n");
                SetColor(DEFAULT);

                QuitFunction();
                return nullptr;
            }

            SkipSpaces();
            SkipEnters();
        }
        p++;

        QuitFunction();
        return new_node;
    }
    else if (shift != 0){   // Var name read. Check if '=' presents

        int tmp_pos = p + shift;
        while(s[tmp_pos] == ' ' || s[tmp_pos] == '\t' || s[tmp_pos] == '\v')
            tmp_pos++;

        if(s[tmp_pos] == '='){     // Assignment found!

            p = tmp_pos + 1;

            printf("\n\n\n\n\n\t\t\t====================\n\n\n\n\n");
            PrintVar(p);

            // Looking for the variable
            int i = 0;
            while(i < n_variables){
                if(!strcmp(word, variables[i])){
                    n_variables++;
                    break;
                }
                i++;
            }

            if(i >= n_variables){
                SetColor(RED);
                printf("=====   %s was not declared in this scope   =====\n", word);
                SetColor(DEFAULT);

                error = true;
                PrintVar(error);

                QuitFunction();
                return nullptr;
            }

            // Counting right operand

            printf("\n\n\n\n\n\t\t\tRIGHT OPERAND BEGIN\n\n\n\n\n");
            PrintVar(p);
            PrintVar(p);
            Node* assign_arg = GetE();
            printf("\n\n\n\n\n\t\t\tRIGHT OPERAND END\n\n\n\n\n");
            PrintVar(p);
            PrintVar(p);
            if(assign_arg == nullptr)
                return nullptr;

            // Creating assignment

            Node* var = Node::CreateNode();
            var->SetDataType        (VARIABLE);
            var->SetData            (i);

            new_node = Node::CreateNode();
            new_node->SetDataType   (OPERATOR);
            new_node->SetData       ('=');
            new_node->SetLeft       (var);
            new_node->SetRight      (assign_arg);

            QuitFunction();
            return new_node;

        }else{                  // Assignment not found

            PrintVar(p);
            DEBUG printf("\n\n\nNO AASIGNMENT\n\n");

            SkipSpaces();
            SkipEnters();

            QuitFunction();
            return GetE();

        }
    }
    else{
        // GetE() must be separated with '\n'
        // If we could get it - ok, else - error

        SkipSpaces();
        while(s[p] == '\n')    p++;

        Node* expr = GetE();

        SkipSpaces();
        if(s[p] != '\n'){
            error = true;
            PrintVar(error);

            QuitFunction();
            return nullptr;
        }

        if(expr == nullptr){
            error = true;
            PrintVar(error);

            QuitFunction();
            return nullptr;
        }

        QuitFunction();
        return expr;
    }
}

Node* GetFunction()
{
    if(error)       return nullptr;

    EnterFunction();

    SkipAllSpaces();
    char word[MAX_VAR_NAME_LEN] = {};

    Node* top_node = nullptr;
    Node* current  = nullptr;

    bool main_read = false;
    while(!main_read){

        int shift  = GetWord(word);

        /*  */if(!strcmp(word, FUNC)){

            p += shift + 1;
            p += GetWord(word);

            int i = 0;
            while(i < n_functions){
                if(!strcmp(word, functions[i])){
                    SetColor(RED);
                    printf("=====   Redeclaration of %s   =====\n", word);
                    SetColor(DEFAULT);

                    error = true;
                    PrintVar(error);

                    QuitFunction();
                    return nullptr;
                }
                i++;
            }

            strcpy(functions[n_functions], word);
            n_functions++;

            SkipSpaces();

            printf("\n\n\n\n\n\t\t\tBLYA\n\n\n\n\n");

            if(s[p] == ':'){            // Function has parameters
                // while(s[p] != '\n')
            }else{                      // Function without parameters

                SkipAllSpaces();
                printf("\n\n\n\n\n\t\t\tBLYA 2\n\n\n\n\n");
                Node* function_body = GetOperator();
                PrintVar(error);

                if(error)       return nullptr;

                Node* new_function = Node::CreateNode();
                new_function->SetDataType   (FUNCTION);
                new_function->SetData       (i);
                new_function->SetRight      (nullptr);
                new_function->SetLeft       (function_body);

                SkipSpaces();

                printf("\n\n\n\n\n\t\t\tBLYA 3\n\n\n\n\n");
                PrintVar(n_functions);

                /*  */if(n_functions == 1){

                printf("top created\n");

                    top_node = Node::CreateNode();
                    top_node->SetDataType   (GLOBAL_NODE);
                    top_node->SetData       (0);
                    top_node->SetRight      (new_function);
                    top_node->SetLeft       (nullptr);

                    SkipSpaces();

                    //QuitFunction();
                    //return top_node;

                }else if(n_functions == 2){

                    printf("second created\n");

                    top_node->SetLeft       (Node::Copy(top_node->GetRight()));
                    Node::DeleteNode        (top_node->GetRight());
                    top_node->SetRight      (new_function);

                    current = top_node->GetRight();

                    SkipSpaces();

                    //QuitFunction();
                    //return top_node;

                }else{

                    printf("third created\n");

                    current->SetLeft(Node::Copy(current));

                    current->SetDataType(GLOBAL_NODE);
                    current->SetData(0);
                    current->SetRight(new_function);

                    current = current->GetRight();

                    SkipSpaces();

                }

            }

        }else if(!strcmp(word, MAIN)){

            main_read = true;

            p += shift;
            Node* main_body = GetOperator();

            if(error)           return nullptr;

            Node* main = Node::CreateNode();
            main->SetDataType           (MAIN_FUNCTION);
            main->SetData               (0);
            main->SetRight              (nullptr);
            main->SetLeft               (main_body);

            if(n_functions == 0){

                printf("just main created\n");

                Node* global_node = Node::CreateNode();
                global_node->SetDataType(GLOBAL_NODE);
                global_node->SetData    (0);
                global_node->SetRight   (main);
                global_node->SetLeft    (nullptr);

                QuitFunction();
                return global_node;

            }else if(n_functions == 1){

                printf("main for 1 created\n");

                    top_node->SetLeft       (Node::Copy(top_node->GetRight()));
                    top_node->SetRight      (main);

                    SkipSpaces();

                    //QuitFunction();
                    //return top_node;

                }

            /*
            {

                //QuitFunction();
                //return main;
            }
            */

        }else{

            //  FUNCTIONS ONLY ALLOWED

            error = true;
            PrintVar(error);

            QuitFunction();
            return nullptr;

        }
    }
}

Node* GetGO(const char* expr)
{
    s = expr;
    p = 0;
    s_len = strlen(expr);

    //Node* top_operand = GetOperator();
    Node* top_operand = GetFunction();
    if(s[p] != '\n' && s[p] != '\0'){
        int error_pos = CountLine();
        SetColor(RED);
        DEBUG printf("G0: Error in line %d\n", error_pos);
        SetColor(DEFAULT);
        PrintVar(p);
        PrintVar(s[p]);
        assert(0);
    }

    if(error)           return nullptr;
    return top_operand;
}



// =========================================    BRAIN


// =============================================================    Atomic print function

/// Prints left branch
/**
    Checks if branch exists before print

    \param [in] output              Output file
    \param [in] left_node           Pointer to the node to be printed
*/
int PrintLeft(FILE* output, Node* left_node)
{
    EnterFunction();

    if(left_node != nullptr)
        TranslateCode(output, left_node);
    else{
        SetColor(RED);
        DEBUG printf("=====   Unexpected nullptr as left branch  =====\n");
        SetColor(DEFAULT);

        QuitFunction();
        return UNEXPECTED_NULLPTR;
    }

    QuitFunction();
    return OK;
}

/// Prints right branch
/**
    Checks if branch exists before print

    \param [in] output              Output file
    \param [in] right_node          Pointer to the node to be printed
*/
int PrintRight(FILE* output, Node* right_node)
{
    EnterFunction();

    if(right_node != nullptr)
        TranslateCode(output, right_node);
    else{
        SetColor(RED);
        DEBUG printf("=====   Unexpected nullptr as left branch  =====\n");
        SetColor(DEFAULT);

        QuitFunction();
        return UNEXPECTED_NULLPTR;
    }

    QuitFunction();
    return OK;
}



int labels_num = 0;

/// Prints if-node
/**
    \param [in] output              Output file
    \param [in] root_node           Pointer to the root
*/
int PrintIf(FILE* output, Node* root_node)
{
    EnterFunction();

    int current_label = labels_num++;

    int right = PrintRight(output, root_node->GetRight());
    fprintf(output, "push 0\nje _%d\n", current_label);
    int left  = PrintLeft (output, root_node->GetLeft());
    fprintf(output, "label _%d\n\n", current_label);

    if(left == UNEXPECTED_NULLPTR || right == UNEXPECTED_NULLPTR){
        PrintVar(left);
        PrintVar(right);

        QuitFunction();
        return UNEXPECTED_NULLPTR;
    }

    //labels_num++;

    QuitFunction();
    return OK;
}

/// Prints until-node
/**
    \param [in] output              Output file
    \param [in] root_node           Pointer to the root
*/
int PrintUntil(FILE* output, Node* root_node)
{
    EnterFunction();

    int current_label = labels_num++;

    fprintf(output, "label _%d\n\n", current_label);
    int left  = PrintLeft (output, root_node->GetLeft());
    int right = PrintRight(output, root_node->GetRight());
    fprintf(output, "push 0\njne _%d\n\n", current_label);

    if(left == UNEXPECTED_NULLPTR || right == UNEXPECTED_NULLPTR){
        PrintVar(left);
        PrintVar(right);

        QuitFunction();
        return UNEXPECTED_NULLPTR;
    }

    //labels_num++;

    QuitFunction();
    return OK;
}

/// Prints assign-node
/**
    \param [in] output              Output file
    \param [in] root_node           Pointer to the root
*/
int PrintAssign(FILE* output, Node* root_node)                                      // ???
{
    EnterFunction();

    if(root_node->GetRight()->GetDataType() == CONSTANT)
        fprintf(output, "push ");
    int right = PrintRight(output, root_node->GetRight());
    fprintf(output, "pop ");
    int left  = PrintLeft (output, root_node->GetLeft());

    if(left == UNEXPECTED_NULLPTR || right == UNEXPECTED_NULLPTR){
        PrintVar(left);
        PrintVar(right);

        QuitFunction();
        return UNEXPECTED_NULLPTR;
    }

    QuitFunction();
    return OK;
}

/// Prints composite-node
/**
    \param [in] output              Output file
    \param [in] root_node           Pointer to the root
*/
int PrintComposite(FILE* output, Node* root_node)
{
    EnterFunction();

    int right = PrintRight(output, root_node->GetRight());
    int left  = PrintLeft (output, root_node->GetLeft());

    if(left == UNEXPECTED_NULLPTR || right == UNEXPECTED_NULLPTR){
        PrintVar(left);
        PrintVar(right);

        QuitFunction();
        return UNEXPECTED_NULLPTR;
    }

    QuitFunction();
    return OK;
}



/// Prints operands for bin-operations
/**
    \param [in] output              Output file
    \param [in] root_node           Pointer to the root
*/
int PrintOperands(FILE* output, Node* root_node)
{
    EnterFunction();

    if (root_node->GetRight()->GetDataType() == CONSTANT ||
        root_node->GetRight()->GetDataType() == VARIABLE)
        fprintf(output, "push ");
    TranslateCode(output, root_node->GetRight());
    if (root_node->GetLeft()->GetDataType() == CONSTANT ||
        root_node->GetLeft()->GetDataType() == VARIABLE)
        fprintf(output, "push ");
    TranslateCode(output, root_node->GetLeft());

    QuitFunction();
    return OK;
}

// =============================================================    Global print functions

/// Prints operator-nodes
/**
    \param [in] output              Output file
    \param [in] root_node           Pointer to the root
*/
int PrintOperator(FILE* output, Node* root_node)
{
    EnterFunction();

    switch((int)root_node->GetData())
    {
        case IF_OP:
        {
            PrintIf         (output, root_node);
            break;
        }
        case UNTIL_OP:
        {
            PrintUntil      (output, root_node);
            break;
        }
        //case ASSIGN_OP:
        case '=':
        {
            PrintAssign     (output, root_node);
            break;
        }
        case COMPOSITE_OP:
        {
            PrintComposite  (output, root_node);
            break;
        }
    }

    QuitFunction();
    return OK;
}

/// Prints bin-operation-nodes
/**
    \param [in] output              Output file
    \param [in] root_node           Pointer to the root
*/
int PrintBinOperation(FILE* output, Node* root_node)
{
    EnterFunction();

    PrintOperands(output, root_node);
    int current_label_1 = labels_num++;
    int current_label_2 = labels_num++;

    switch((int)root_node->GetData())
    {
        case '+':
        {
            fprintf(output, "add\n");
            break;
        }
        case '-':
        {
            fprintf(output, "sub\n");
            break;
        }
        case '*':
        {
            fprintf(output, "mul\n");
            break;
        }
        case '/':
        {
            fprintf(output, "div\n");
            break;
        }
        case '>':
        {
            fprintf(output, "ja _%d\n", current_label_1);
            fprintf(output, "push 0\n");
            fprintf(output, "push 0\n");
            fprintf(output, "push 0\n");
            fprintf(output, "je _%d\n", current_label_2);
            fprintf(output, "label _%d\n", current_label_1);
            fprintf(output, "push 1\n");
            fprintf(output, "label _%d\n", current_label_2);
            break;
        }
        case '<':
        {
            fprintf(output, "jb _%d\n", current_label_1);
            fprintf(output, "push 0\n");
            fprintf(output, "push 0\n");
            fprintf(output, "push 0\n");
            fprintf(output, "je _%d\n", current_label_2);
            fprintf(output, "label _%d\n", current_label_1);
            fprintf(output, "push 1\n");
            fprintf(output, "label _%d\n", current_label_2);
            break;
        }
        case '~':
        {
            fprintf(output, "je _%d\n", current_label_1);
            fprintf(output, "push 0\n");
            fprintf(output, "push 0\n");
            fprintf(output, "push 0\n");
            fprintf(output, "je _%d\n", current_label_2);
            fprintf(output, "label _%d\n", current_label_1);
            fprintf(output, "push 1\n");
            fprintf(output, "label _%d\n", current_label_2);
            break;
        }
    }

    QuitFunction();
    return OK;
}

int TranslateCode(FILE* output, Node* root_node)
{
    EnterFunction();

    PrintVar(root_node->GetDataType());
    switch(root_node->GetDataType())
    {
        case OPERATOR:
        {
            PrintOperator       (output, root_node);
            break;
        }
        case BIN_OPERATION:
        {
            PrintBinOperation   (output, root_node);
            break;
        }
        case VARIABLE_TO_CREATE:
        {
            // Now nothing. Lately - create new, i.e. something related to
            break;
        }
        case VARIABLE:
        {
            fprintf(output, " [%d]\n", (int)root_node->GetData());
            break;
        }
        case CONSTANT:
        {
            fprintf(output, "% lg\n", root_node->GetData());
            break;
        }
    }

    QuitFunction();
    return OK;
}

int BuildSyntaxTree(Tree* tree)
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

int CompileCode()
{
    FILE* output = fopen(ASM_CODE, "w");
    assert(output != nullptr);

    Tree tree;

    BuildSyntaxTree(&tree);
    tree.CallGraph();
    TranslateCode(output, tree.GetRoot());
}



