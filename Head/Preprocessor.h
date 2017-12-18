#ifndef PREPROCESSOR_H_INCLUDED
#define PREPROCESSOR_H_INCLUDED

#include <assert.h>
#include "Errors.h"

#define _DEBUG
#include "DebugLib.h"

// =========================================

#define MAX_INSTRUCTION_LEN 100

#define PLUG "plug"


// =========================================


/// Returns amount of chars read
/**
    \param [in]  filename               Name of the file to be read
    \param [out] dest                   Buffer to be filled

    \warning Returns -1 in case of error
*/
int FileRead(const char* file_name, char*& dest);

/// Places file to be inserted to the main
/**
    \param [in] input                   Input  file
    \param [in] output                  Output file
    \param [in] place_in_output         Place from where we should start writing
*/
int Plug(const char* input_name, FILE* output);

/// Gets word from the string
/**
    \param [out] word                   Buffer to be filled
*/
int GetInstruction(const char* input_str, int from, char* word);

/// Provides with preprocessing
/**
    Replaces code instructions which begin with $ with related commands

    \param [in] input                   Input  file
    \param [in] output                  Output file
*/
int Preprocessor(const char* input_name, const char* output_name);

#endif // PREPROCESSOR_H_INCLUDED
