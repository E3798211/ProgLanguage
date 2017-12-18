#include "../Head/Preprocessor.h"

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
}

// =====================================================

#define TMP "tmp "
const int tmp_len           = 5;
char      num               = 'a';
const int param             = 3;        // empty space in \tmp_file_name

char* GetUniqueTmpName()
{
    char* unique_tmp = nullptr;
    try
    {
        unique_tmp = new char [tmp_len];
    }
    catch(std::bad_alloc& ex)
    {
        SetColor(RED);
        DEBUG printf("=====   Cannot allocate %d bytes   =====\n", tmp_len);
        SetColor(DEFAULT);

        QuitFunction();
        return nullptr;
    }

    strcpy(unique_tmp, TMP);
    unique_tmp[param] = num++;

    return unique_tmp;
}

char* DeleteUniqueTmpName(char* name_to_delete)
{
    delete [] name_to_delete;
    num--;
}

// =====================================================

int GetInstruction(const char* input_str, int from, char* word)
{
    EnterFunction();

    int input_cur_pos = from;
    while(input_str[input_cur_pos] == ' ' || input_str[input_cur_pos] == '\t' || input_str[input_cur_pos] == '\v' ||
          input_str[input_cur_pos] == '\0')
        input_cur_pos++;

    int i = 0;
    while(!isspace(input_str[input_cur_pos])){
        if(i < MAX_INSTRUCTION_LEN)
            word[i++] = input_str[input_cur_pos++];
    }

    word[i] = '\0';
    QuitFunction();
    return input_cur_pos;
}

int Preprocessor(const char* input_name, const char* output_name)
{
    EnterFunction();

    char* input_str     = nullptr;
    int min_output_len  = FileRead(input_name, input_str);

    int   input_cur_pos = 0;

    if(input_str == nullptr){
        SetColor(RED);
        DEBUG printf("=====   Can not open \"%s\"   =====\n", input_name);
        SetColor(DEFAULT);

        QuitFunction();
        return FILE_NOT_OPENED;
    }

    FILE* output_file = fopen(output_name, "w");

    char word[MAX_INSTRUCTION_LEN] = {};

    input_cur_pos = 0;
    while(input_str[input_cur_pos] != '\0'){

        if(input_str[input_cur_pos] == '$'){
            input_cur_pos++;

            input_cur_pos = GetInstruction(input_str, input_cur_pos, word);

            /* */if(!strcmp(word, PLUG)){

                input_cur_pos = GetInstruction(input_str, input_cur_pos, word);
                Plug(word, output_file);
            }
            else{
                SetColor(RED);
                printf("=====   Unknown instruction: \"%s\"   =====\n", word);
                printf("=====   \"%s\": instruction ignored   =====\n", word);
                SetColor(DEFAULT);
            }

            // If instruction met we go further without printing - what if next char is '$'?
            continue;
        }

        fprintf(output_file, "%c", input_str[input_cur_pos++]);
    }

    fprintf(output_file, "%c", ' ');
    delete [] input_str;
    fclose(output_file);

    QuitFunction();
    return OK;
}

int Plug(const char* input_name, FILE* output)
{
    EnterFunction();

    char* to_plug_in = nullptr;
    FileRead(input_name, to_plug_in);

    // Creating tmp files with new unique names
    char* raw_tmp           = GetUniqueTmpName();
    char* preprocessed_tmp  = GetUniqueTmpName();

    // Writing to the raw_tmp
    FILE* raw_output = fopen(raw_tmp, "w");

    int i = 0;
    while(to_plug_in[i] != '\0')
        fprintf(raw_output, "%c", to_plug_in[i++]);
    fprintf(raw_output, "%c", ' ');

    fclose(raw_output);

    // Preprocessing it
    Preprocessor(raw_tmp, preprocessed_tmp);

    // Replacing all back to the output
    delete [] to_plug_in;
    to_plug_in = nullptr;
    FileRead(preprocessed_tmp, to_plug_in);

    i = 0;
    while(to_plug_in[i] != '\0')
        fprintf(output, "%c", to_plug_in[i++]);

    delete [] to_plug_in;
    remove(raw_tmp);
    remove(preprocessed_tmp);
    DeleteUniqueTmpName(raw_tmp);
    DeleteUniqueTmpName(preprocessed_tmp);

    QuitFunction();
    return OK;
}



