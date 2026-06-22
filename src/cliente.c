#include "powerudp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* =========================================================
 * Uso:
 *   ./cliente <ip_servidor> <psk>
 * ========================================================= */
int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Uso: %s <ip_servidor> <psk>\n", argv[0]);
        exit(1);
    }

    const char *server_ip = argv[1];
    const char *psk       = argv[2];

    /* --- 1. Inicializar protocolo e registar no servidor --- */
    if (init_protocol(server_ip, SERVER_TCP_PORT, psk) < 0) {
        fprintf(stderr, "[cliente] falha na inicializacao\n");
        exit(1);
    }

    /* --- 2. Menu interactivo --- */
    char input[512];
    char dest[64];

    printf("\nComandos disponiveis:\n");
    printf("  send <ip_destino> <mensagem>  - envia mensagem PowerUDP\n");
    printf("  recv                          - aguarda mensagem\n");
    printf("  config <retx> <backoff> <seq> <timeout_ms> <max_retries>\n");
    printf("  loss <probabilidade_0_100>    - injectar perda de pacotes\n");
    printf("  stats                         - ver estatisticas ultima msg\n");
    printf("  sair                          - terminar\n\n");

    while (1) {
        printf("> ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin)) break;
        /* Remover newline */
        input[strcspn(input, "\n")] = '\0';

        if (strncmp(input, "send ", 5) == 0) {
            /* send <ip> <mensagem> */
            char *rest = input + 5;
            char *space = strchr(rest, ' ');
            if (!space) { printf("Uso: send <ip> <mensagem>\n"); continue; }
            *space = '\0';
            strncpy(dest, rest, sizeof(dest) - 1);
            char *msg = space + 1;
            printf("[cliente] A enviar para %s: \"%s\"\n", dest, msg);
            if (send_message(dest, msg, (int)strlen(msg)) == 0) {
                int retx, dtime;
                get_last_message_stats(&retx, &dtime);
                printf("[cliente] Entregue: retransmissoes=%d  tempo=%dms\n",
                       retx, dtime);
            } else {
                printf("[cliente] Falha no envio\n");
            }

        } else if (strcmp(input, "recv") == 0) {
            char buf[POWERUDP_BUF_SIZE];
            printf("[cliente] A aguardar mensagem...\n");
            int n = receive_message(buf, sizeof(buf));
            if (n > 0)
                printf("[cliente] Recebido (%d bytes): \"%s\"\n", n, buf);
            else
                printf("[cliente] Erro na recepcao\n");

        } else if (strncmp(input, "config ", 7) == 0) {
            /* config <retx> <backoff> <seq> <timeout> <max_retries> */
            int retx, backoff, seq, timeout, retries;
            if (sscanf(input + 7, "%d %d %d %d %d",
                       &retx, &backoff, &seq, &timeout, &retries) == 5) {
                if (request_protocol_config(retx, backoff, seq,
                                            (uint16_t)timeout,
                                            (uint8_t)retries) == 0)
                    printf("[cliente] Pedido de config enviado ao servidor\n");
                else
                    printf("[cliente] Erro ao enviar pedido de config\n");
            } else {
                printf("Uso: config <retx 0|1> <backoff 0|1> <seq 0|1> "
                       "<timeout_ms> <max_retries>\n");
            }

        } else if (strncmp(input, "loss ", 5) == 0) {
            int prob = atoi(input + 5);
            inject_packet_loss(prob);

        } else if (strcmp(input, "stats") == 0) {
            int retx, dtime;
            get_last_message_stats(&retx, &dtime);
            printf("[stats] retransmissoes=%d  tempo_entrega=%dms\n", retx, dtime);

        } else if (strcmp(input, "sair") == 0) {
            break;

        } else if (strlen(input) > 0) {
            printf("Comando desconhecido: %s\n", input);
        }
    }

    close_protocol();
    return 0;
}
