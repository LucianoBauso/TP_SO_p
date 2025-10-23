#ifndef QUERY_INTERPRETER_H_
#define QUERY_INTERPRETER_H_

typedef enum {
    IN_CREATE,
    IN_TRUNCATE,
    IN_WRITE,
    IN_READ,
    IN_TAG,
    IN_COMMIT,
    IN_FLUSH,
    IN_DELETE,
    IN_END,
    IN_INVALIDA // En caso d error
} t_instruccion_worker;

#endif