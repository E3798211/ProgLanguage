#include "../Head/ProgLanguage.h"

// =========================================    SUPPORTING FUNCTIONS


/// Returns amount of chars read
/**
    \param [in]  filename               Name of the file to be read
    \param [out] dest                   Buffer to be filled

    \warning Returns -1 in case of error
*/
/*
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
}*/

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
#define RET     "return"
#define IF      "if"
#define UNTIL   "until"
#define VAR     "var"
#define ASM     "asm"


#define MAX_VAR_AMNT        1000
#define MAX_VAR_NAME_LEN    100
char variables[MAX_VAR_AMNT][MAX_VAR_NAME_LEN] = {};
int  n_variables = 0;
int  var_num_in_function = 0;

#define MAX_FUNC_AMNT       1000
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
int  GetWord(char* word)
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
bool VarExists(char* word)
{
    int i = 0;
    //while(i < n_variables){
    while(i < var_num_in_function){
        if(!strcmp(word, variables[i]))
            return true;
        i++;
    }
    return false;
}
bool FuncExists(char* word)
{
    int i = 0;
    while(i < n_functions){
        if(!strcmp(word, functions[i])){
            return true;
        }
        i++;
    }
    return false;
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

        /* */if(VarExists(word)){

            int i = 0;
            //while(strcmp(word, variables[i]))
            //    i++;
            while(i <= var_num_in_function){
                if(!strcmp(variables[i], word)){
                    break;
                    i++;
                }
                i++;
            }

            Node* new_node = Node::CreateNode();
            new_node->SetDataType(VARIABLE);
            new_node->SetData(i + 1);

            SkipSpaces();

            QuitFunction();
            return new_node;

        }
        else if(FuncExists(word)){

            Node* first_arg = nullptr;
            Node* current   = nullptr;
            int n_arguments = 0;

            SkipSpaces();

            // Looking for arguments
            if(s[p] == '['){

                p++;
                SkipSpaces();
                while(s[p] != ']'){

                    SkipSpaces();
                    Node* arg = GetE();

                    Node* next_arg = Node::CreateNode();
                    next_arg->SetDataType       (CALL_ARGUMENT_LIST);
                    next_arg->SetData           (n_arguments);
                    next_arg->SetLeft           (nullptr);
                    next_arg->SetRight          (arg);

                    /* */if(n_arguments == 0){
                        first_arg = next_arg;
                        current   = first_arg;
                    }
                    else{
                        current->SetLeft        (next_arg);
                        current = current->GetLeft();
                    }

                    n_arguments++;

                    SkipSpaces();
                    if(s[p] == '|')     p++;
                    SkipSpaces();
                }
                // Skipping ']'
                p++;

            }else{

                SetColor(RED);
                printf("=====   Expected brackets after %s   =====\n", word);
                SetColor(DEFAULT);

                error = true;
                PrintVar(error);

                return nullptr;

            }


            int i = 0;
            while(!strcmp(word, variables[i]))
                i++;

            Node* new_node = Node::CreateNode();
            new_node->SetDataType   (CALL_FUNCTION);
            new_node->SetData       (i);
            new_node->SetRight      (first_arg);
            new_node->SetLeft       (nullptr);

            SkipSpaces();

            QuitFunction();
            return new_node;

        }
        else{

            SetColor(RED);
            printf("=====   %s was not declared in this scope   =====\n", word);
            SetColor(DEFAULT);

            error = true;
            PrintVar(error);

            return nullptr;

        }
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
        //while(i < n_variables){
        while(i < var_num_in_function){
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

        //strcpy(variables[n_variables], word);
        strcpy(variables[var_num_in_function], word);
        n_variables++;

        Node* var = Node::CreateNode();
        var->SetDataType(VARIABLE_TO_CREATE);
        //var->SetData    (i);
        var->SetData    (var_num_in_function + 1);
        var_num_in_function++;

        QuitFunction();
        return var;

    }
    else if(!strcmp(word, RET)){

        SkipSpaces();
        p += shift;

        Node* to_be_returned = GetE();
        if(error)                       return nullptr;

        Node* return_statement = Node::CreateNode();
        return_statement->SetDataType   (RETURN);
        return_statement->SetData       (0);
        return_statement->SetLeft       (nullptr);
        return_statement->SetRight      (to_be_returned);

        QuitFunction();
        return return_statement;

    }
    else if(!strcmp(word, ASM)){

        SkipSpaces();
        p += shift;
        SkipAllSpaces();
        if(s[p] == '<'){

            int beg = ++p;

            Node* asm_beg    = Node::CreateNode();
            asm_beg->SetDataType        (ASM_CODE_BEG);
            asm_beg->SetData            (beg);

            Node* asm_insert = Node::CreateNode();
            asm_insert->SetDataType     (OPERATOR);
            asm_insert->SetData         (ASM_INSERT);
            asm_insert->SetRight        (asm_beg);

            while(s[p] != '>')  p++;
            p++;

            return asm_insert;

        }else{
            error = true;
            PrintVar(error);

            QuitFunction();
            return nullptr;
        }

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
        while(/*s[p] != '}'*/1){

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
    else if (shift != 0){   // Var name read. Check if '=' presents                                     // or function call

        int tmp_pos = p + shift;
        while(s[tmp_pos] == ' ' || s[tmp_pos] == '\t' || s[tmp_pos] == '\v')
            tmp_pos++;

        if(s[tmp_pos] == '='){     // Assignment found!

            p = tmp_pos + 1;

            // Looking for the variable
            int i = 0;
            //while(i < n_variables){
            while(i < var_num_in_function){
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

            Node* assign_arg = GetE();
            if(assign_arg == nullptr)
                return nullptr;

            // Creating assignment

            Node* var = Node::CreateNode();
            var->SetDataType        (VARIABLE);
            //var->SetData            (i);
            var->SetData            (i + 1);

            new_node = Node::CreateNode();
            new_node->SetDataType   (OPERATOR);
            new_node->SetData       ('=');
            new_node->SetLeft       (var);
            new_node->SetRight      (assign_arg);

            QuitFunction();
            return new_node;

        }else{                  // Assignment not found

            SkipAllSpaces();

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

// =============================================

/**
    Requires p set to the beginning of the body
*/
Node* GetFunctionBody()
{
    EnterFunction();

    SkipAllSpaces();
    Node* body = GetOperator();
    if(body == nullptr){
        error = true;
        PrintVar(error);

        QuitFunction();
        return nullptr;
    }

    QuitFunction();
    return body;
}

Node* GetMain()
{
    EnterFunction();

    Node* body = GetFunctionBody();
    if(error)           return nullptr;

    Node* main = Node::CreateNode();
    main->SetDataType   (MAIN_FUNCTION);
    main->SetData       (0);
    main->SetLeft       (body);
    main->SetRight      (nullptr);

    QuitFunction();
    return main;
}

Node* GetSingleFunction()
{
    EnterFunction();

    // Having p set to the beginning of the name
    char word[MAX_FUNC_NAME_LEN] = {};

    Node* func      = nullptr;
    Node* first_arg = nullptr;
    Node* current   = nullptr;

    SkipSpaces();
    p += GetWord(word);


    if(FuncExists(word)){
        SetColor(RED);
        printf("=====   Redeclaration of %s   =====\n", word);
        SetColor(DEFAULT);

        error = true;
        PrintVar(error);

        QuitFunction();
        return nullptr;
    }

    strcpy(functions[n_functions++], word);

    SkipSpaces();
    /* */if(s[p] == ':'){       // Function with arguments

        p++;

        int n_arguments = 0;
        while(s[p] != '\n'){

            SkipSpaces();
            p += GetWord(word);
            PrintVar(p);

            // Checking if such variable already exists

            if(VarExists(word)){
                SetColor(RED);
                printf("=====   Redeclaration of %s   =====\n", word);
                SetColor(DEFAULT);

                error = true;
                PrintVar(error);

                QuitFunction();
                return nullptr;
            }

            strcpy(variables[n_variables], word);
            //int arg_num = n_variables;
            int arg_num = var_num_in_function + 1;
            var_num_in_function++;
            n_variables++;

            // Creating new argument

            Node* arg = Node::CreateNode();
            arg->SetDataType            (ARGUMENT);
            arg->SetData                (arg_num);

            Node* next_arg = Node::CreateNode();
            next_arg->SetDataType       (DECLARE_ARGUMENT_LIST);
            next_arg->SetData           (n_arguments);
            next_arg->SetLeft           (nullptr);
            next_arg->SetRight          (arg);

            if(n_arguments == 0){
                first_arg = next_arg;
                current   = first_arg;
            }else{
                current->SetLeft        (next_arg);
                current = current->GetLeft();
            }

            n_arguments++;
            SkipSpaces();
        }
        SkipAllSpaces();

        Node* body = GetFunctionBody();
        if(error)           return nullptr;

        func = Node::CreateNode();
        func->SetDataType   (DECLARE_FUNCTION);
        func->SetData       (n_functions - 1);
        func->SetLeft       (body);
        func->SetRight      (first_arg);

    }
    else if(s[p] == '\n'){      // Function with no parameters

        Node* body = GetFunctionBody();
        if(error)           return nullptr;

        func = Node::CreateNode();
        func->SetDataType   (DECLARE_FUNCTION);
        func->SetData       (n_functions - 1);
        func->SetLeft       (body);
        func->SetRight      (nullptr);

    }
    else{                       // Invalid input
        error = true;
        PrintVar(error);

        QuitFunction();
        return nullptr;
    }

    // After creating function, set to zero variable counter
    var_num_in_function = 0;

    QuitFunction();
    return func;
}

// =============================================

Node* GetFunctions()
{
    if(error)       return nullptr;

    EnterFunction();

    SkipAllSpaces();
    char word[MAX_FUNC_NAME_LEN] = {};

    Node* first   = nullptr;
    Node* current = nullptr;

    bool main_read = false;
    while(!main_read){

        SkipAllSpaces();
        p += GetWord(word);

        /*  */if(!strcmp(word, FUNC)){

            SkipSpaces();
            Node* func = GetSingleFunction();
            //MARK
            //PrintVar(p);
            //PrintVar(s[p]);
            PrintVar(n_functions);
            if(error)               return nullptr;

            /* */if(n_functions == 1){

                first = Node::CreateNode();
                first->SetDataType  (GLOBAL_NODE);
                first->SetData      (0);
                first->SetLeft      (nullptr);
                first->SetRight     (func);

            }
            else if(n_functions == 2){

                first->SetLeft      (Node::Copy(first->GetRight()));
                Node::DeleteNode(first->GetRight());
                first->SetRight     (func);

                current = first->GetRight();

            }
            else{

                current->SetLeft    (Node::Copy(current));
                current->SetDataType(GLOBAL_NODE);
                current->SetData    (0);
                current->SetRight   (func);

                current = current->GetRight();

            }

            //n_functions++;

        }
        else if(!strcmp(word, MAIN)){

            main_read = true;

            Node* main = GetMain();
            if(error)               return nullptr;

            /*  */if(n_functions == 0){

                first = Node::CreateNode();
                first->SetDataType  (GLOBAL_NODE);
                first->SetData      (0);
                first->SetLeft      (nullptr);
                first->SetRight     (main);

            }else if(n_functions == 1){

                first->SetLeft      (Node::Copy(first->GetRight()));

                PrintVar(first->GetRight());

                Node::DeleteNode    (first->GetRight());
                first->SetRight     (main);

            }else{

                current->SetLeft    (Node::Copy(current));

                current->SetDataType(GLOBAL_NODE);
                current->SetData    (0);
                current->SetRight   (main);

                current = current->GetRight();

            }

        }else{

            // Only \MAIN and \function allowed

            error = true;
            PrintVar(error);

            QuitFunction();
            return nullptr;

        }
    }

    return first;
}

Node* GetGO(const char* expr)
{
    s = expr;
    p = 0;
    s_len = strlen(expr);

    //Node* top_operand = GetOperator();
    Node* top_operand = GetFunctions();
    if(s[p] != '\n' && s[p] != '\0'){
        int error_pos = CountLine();
        SetColor(RED);
        printf("G0: Error in line %d\n", error_pos);
        SetColor(DEFAULT);
        PrintVar(p);
        PrintVar(s[p]);
        assert(0);
    }

    if(error){
        int error_pos = CountLine();
        SetColor(RED);
        printf("G0: Error in line %d\n", error_pos);
        SetColor(DEFAULT);
        PrintVar(p);
        PrintVar(s[p]);
        assert(0);
    }
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
        TranslateCode_2_lvl(output, left_node);
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
        TranslateCode_2_lvl(output, right_node);
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

    /* */if(root_node->GetRight()->GetDataType() == VARIABLE)
        fprintf(output, "push ax \npush %d \nadd \npop cx \npush [cx]\n", (int)root_node->GetRight()->GetData());
    else if(root_node->GetRight()->GetDataType() == CONSTANT)
        fprintf(output, "push %lg\n", root_node->GetRight()->GetData());
    else
        /*int right = */PrintRight(output, root_node->GetRight());
    /*
    fprintf(output, "pop ");
    int left  = PrintLeft (output, root_node->GetLeft());
    */
    fprintf(output, "push ax \npush %d \nadd \npop cx \npop [cx]\n", (int)root_node->GetLeft()->GetData());

    /*
    if(/*left == UNEXPECTED_NULLPTR || *right == UNEXPECTED_NULLPTR){
        /*PrintVar(left);*
        PrintVar(right);

        QuitFunction();
        return UNEXPECTED_NULLPTR;
    }
    */

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

    /*
    if (root_node->GetRight()->GetDataType() == CONSTANT ||
        root_node->GetRight()->GetDataType() == VARIABLE)
        fprintf(output, "push ");
    TranslateCode_2_lvl(output, root_node->GetRight());
    if (root_node->GetLeft()->GetDataType() == CONSTANT ||
        root_node->GetLeft()->GetDataType() == VARIABLE)
        fprintf(output, "push ");
    TranslateCode_2_lvl(output, root_node->GetLeft());
    */
    if (root_node->GetRight()->GetDataType() == CONSTANT)
        fprintf(output, "push ");
    if (root_node->GetRight()->GetDataType() == VARIABLE)
        fprintf(output, "push ax \npush %d \nadd \npop cx \npush [cx]\n", (int)root_node->GetRight()->GetData());
    TranslateCode_2_lvl(output, root_node->GetRight());

    if (root_node->GetLeft()->GetDataType() == CONSTANT)
        fprintf(output, "push ");
    if (root_node->GetLeft()->GetDataType() == VARIABLE)
        fprintf(output, "push ax \npush %d \nadd \npop cx \npush [cx]\n", (int)root_node->GetLeft()->GetData());
    TranslateCode_2_lvl(output, root_node->GetLeft());

    QuitFunction();
    return OK;
}

/// Prints asm-node
/**
    \param [in] output              Output file
    \param [in] root_node           Pointer to the root
*/
int PrintAsmInsert(FILE* output, Node* root_node)
{
    EnterFunction();

    int i = (int)root_node->GetRight()->GetData();
    while(s[i] != '>'){
        fprintf(output, "%c", s[i++]);
    }
    fprintf(output, "\n");

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
        case ASM_INSERT:
        {
            PrintAsmInsert  (output, root_node);
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
/*
/// Prints variable-nodes
/**
    \param [in] output              Output file
    \param [in] root_node           Pointer to the root
/
int PrintVariable(FILE* output, Node* root_node)
{
    EnterFunction();

    //fprintf(output, "push ax \npush %d \nadd \npop cx");

    QuitFunction();
    return OK;
}
*/

/// Prints call-function nodes
/**
    \param [in] output              Output file
    \param [in] root_node           Pointer to the root
*/
int PrintCallFunction(FILE* output, Node* root_node)
{
    EnterFunction();

    //fprintf(output, "\n\npush ax \npop [bx] \npush bx \npop ax \npush bx \npush 1 \nadd \npop bx\n");
    fprintf(output, "\n\npush ax \npop [bx] \npush bx \npop dx \npush bx \npush 1 \nadd \npop bx\n");
    if(root_node->GetRight() != nullptr)        // arguments will be printed if they present
        TranslateCode_2_lvl(output, root_node->GetRight());
    fprintf(output, "\npush dx \npop ax\n");
    fprintf(output, "call _%s\n", functions[(int)root_node->GetData()]);

    QuitFunction();
    return OK;
}

/// Prints argument list nodes
/**
    \param [in] output              Output file
    \param [in] root_node           Pointer to the root
*/
int PrintCallArgList(FILE* output, Node* root_node)
{
    EnterFunction();

    //fprintf(output, "push ");

    // Here we believe that variable or constant is on the right
    /* */if(root_node->GetRight()->GetDataType() == CONSTANT)
        fprintf(output, "push %lg\n", root_node->GetRight()->GetData());
    else if(root_node->GetRight()->GetDataType() == VARIABLE)
        fprintf(output, "push ax \npush %d \nadd \npop cx \npush [cx]\n", (int)root_node->GetRight()->GetData());
    else
        TranslateCode_2_lvl(output, root_node->GetRight());

    //PrintRight(output, root_node->GetRight());      // Here we believe that variable or constant is on the right

    fprintf(output, "pop [bx] \npush 1 \npush bx \nadd \npop bx\n");
    if(root_node->GetLeft() != nullptr)
        TranslateCode_2_lvl(output, root_node->GetLeft());

    QuitFunction();
    return OK;
}

/// Prints return-nodes
/**
    \param [in] output              Output file
    \param [in] root_node           Pointer to the root
*/
int PrintReturn(FILE* output, Node* root_node)
{
    EnterFunction();

    /* */if(root_node->GetRight()->GetDataType() == CONSTANT)
        fprintf(output, "push %lg\n", root_node->GetRight()->GetData());
    else if(root_node->GetRight()->GetDataType() == VARIABLE)
        fprintf(output, "push ax \npush %d \nadd \npop cx \npush [cx]\n", (int)root_node->GetRight()->GetData());
    else
        TranslateCode_1_lvl(output, root_node->GetRight());

    fprintf(output, "\npush ax \npop bx \npush [ax] \npop ax \nret\n\n");

    QuitFunction();
    return OK;
}

int TranslateCode_2_lvl(FILE* output, Node* root_node)
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
            // Nothing
            fprintf(output, "push bx \npush 1 \nadd \npop bx\n");
            break;
        }
        case VARIABLE:
        {
            //fprintf(output, " [%d]\n", (int)root_node->GetData());
            // All prints are already done before we ask for it.
            break;
        }
        case CONSTANT:
        {
            fprintf(output, "% lg\n", root_node->GetData());
            break;
        }
        case CALL_FUNCTION:
        {
            PrintCallFunction(output, root_node);
            break;
        }
        case CALL_ARGUMENT_LIST:
        {
            PrintCallArgList(output, root_node);
            break;
        }
        case ARGUMENT:
        {
            // Nothing
            break;
        }
        case RETURN:
        {
            PrintReturn(output, root_node);
            break;
        }
    }

    QuitFunction();
    return OK;
}

// =========================================================

/// Prints high nodes
/**
    \param [in] output              Output file
    \param [in] root_node           Pointer to the root
*/
int PrintGlobal(FILE* output, Node* root_node)
{
    EnterFunction();

    if(root_node->GetRight() != nullptr)
        TranslateCode_1_lvl(output, root_node->GetRight());
    if(root_node->GetLeft()  != nullptr)
        TranslateCode_1_lvl(output, root_node->GetLeft());

    QuitFunction();
    return OK;
}

/// Prints declare-func nodes
/**
    \param [in] output              Output file
    \param [in] root_node           Pointer to the root
*/
int PrintDeclareFunction(FILE* output, Node* root_node)
{
    EnterFunction();

    fprintf(output, "label _%s\n", functions[(int)root_node->GetData()]);
    TranslateCode_2_lvl(output, root_node->GetLeft());
    fprintf(output, "\npush ax \npop bx \npush [ax] \npop ax \nret\n\n");

    QuitFunction();
    return OK;
}

int TranslateCode_1_lvl(FILE* output, Node* root_node)
{
    EnterFunction();

    switch(root_node->GetDataType())
    {
        case GLOBAL_NODE:
        {
            PrintGlobal(output, root_node);
            break;
        }
        case DECLARE_FUNCTION:
        {
            PrintDeclareFunction(output, root_node);
            break;
        }
        case MAIN_FUNCTION:
        {
            fprintf(output, "push 1\n pop bx\n");
            TranslateCode_2_lvl(output, root_node->GetLeft());
            fprintf(output, "end\n\n\n");
            break;
        }
        case DECLARE_ARGUMENT_LIST:
        {
            // Nothing
            break;
        }
        default:
        {
            TranslateCode_2_lvl(output, root_node);
        }

    }

    QuitFunction();
    return OK;
}






int BuildSyntaxTree(Tree* tree)
{
    EnterFunction();

    // PREPROCESSING

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

    // BuildSyntaxTree(&tree);
    // tree.CallGraph();
    // TranslateCode_2_lvl(output, tree.GetRoot());
    // return TranslateCode_1_lvl(output, tree.GetRoot());
}


