#ifndef PROTOCOL_H_
#define PROTOCOL_H_

typedef enum {
    MENSAJE = 1,
    PAQUETE,
    CREATE_FILE,
    TRUNCATE_FILE,
    WRITE_FILE,
    READ_FILE,
    TAG_FILE,
    FILE_CONTENT,
    READ_RESULT,
    EXECUTE_QUERY,
    COMMIT_FILE,
    FLUSH_FILE,
    DELETE_FILE,
    QUERY_OK,
    QUERY_FINISHED
} op_code;

#endif /* PROTOCOL_H_ */
