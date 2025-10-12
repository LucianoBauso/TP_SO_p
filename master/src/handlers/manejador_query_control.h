#ifndef MANEJADOR_QUERY_CONTROL_H
#define MANEJADOR_QUERY_CONTROL_H

void registrar_query_control(int socket_query_control);
void remover_query_control_y_cancelar(int socket_query_control);
void manejar_query_control(int socket_query_control);

#endif
