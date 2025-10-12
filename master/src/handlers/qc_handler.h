#ifndef QC_HANDLER_H
#define QC_HANDLER_H

void registrar_qc(int qc_socket);
void remover_qc_y_cancelar(int qc_socket);
void manejar_query_control(int qc_socket);

#endif
