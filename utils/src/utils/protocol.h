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
    READ_RESULT
} op_code;

#endif /* PROTOCOL_H_ */
