#ifndef ERRORS_H_INCLUDED
#define ERRORS_H_INCLUDED

enum ErrorCodes {
    OPTIMIZATION_FAILED = -9,

    UNIDENTIFIED_DATA   = -8,

    ALLOC_ERROR         = -7,

    FILE_NOT_OPENED     = -6,

    DATA_NOT_CREATED    = -5,

    NODE_DOES_NOT_EXIST = -4,
    NODE_ALREADY_EXIST  = -3,
    NODE_NOT_CREATED    = -2,

    INVALID_ARGUMENT    = -1,
    OK = 0
};

#endif // ERRORS_H_INCLUDED
