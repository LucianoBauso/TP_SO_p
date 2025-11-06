#ifndef PROTOCOL_H_
#define PROTOCOL_H_

typedef enum {
    MENSAJE = 1,
    PAQUETE,
    CREATE_FILE,
    TRUNCATE_FILE
} op_code;

#endif /* PROTOCOL_H_ */
