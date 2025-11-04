#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>

void registrar_evento(const char *filename, const char *event) {
    FILE *file = fopen(filename, "a"); // adiciona no final do arquivo sem afetar o anterior (modo append)
    if (!file) {
        perror("Erro ao abrir arquivo de log");
        return;
    }

    // Adiciona timestamp
    time_t agora = time(NULL);
    char *time_str = ctime(&agora);
    time_str[strlen(time_str) - 1] = '\0'; // remove '\n'

    fprintf(file, "[%s] %s\n", time_str, event);
    fclose(file);
}

int main() {
    int pipeline[2];  // pipeline[0] = leitura, pipeline[1] = escrita
    pid_t pid;
    const char *registro = "programa_log.txt";

    // Cria o pipe
    if (pipe(pipeline) == -1) {
        perror("Erro ao criar o pipe");
        exit(EXIT_FAILURE);
    }

    // Cria o processo filho
    pid = fork();

    if (pid < 0) {
        perror("Erro ao criar o processo");
        exit(EXIT_FAILURE);
    }

    // Processo filho é CONSUMIDOR, então ele não escreve
    if (pid == 0) {
        close(pipeline[1]); // Fecha a extremidade de escrita

        char buffer[256];
        ssize_t bytes_lidos;

        printf("Consumidor (filho) iniciado, aguardando mensagens...\n");
        registrar_evento(registro, "Consumidor iniciado.");

        // Lê continuamente até fechar o pipe
        int n_mensagem = 1;
        while ((bytes_lidos = read(pipeline[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes_lidos] = '\0'; // Garante fim de string
            printf("{Mensagem %d} Mensagem recebida: %s\n", n_mensagem, buffer);

            char logmsg[300];
            snprintf(logmsg, sizeof(logmsg), "{Mensagem %d} Mensagem recebida: %s", n_mensagem, buffer);
            registrar_evento(registro, logmsg);
            n_mensagem ++;
        }

        close(pipeline[0]);
        printf("Consumidor encerrado.\n");
        registrar_evento(registro, "Consumidor encerrado.");
        exit(EXIT_SUCCESS);
    }

    // Processo pai é PRODUTOR, então ele não lê
    else {
        close(pipeline[0]); // Fecha a extremidade de leitura

        char mensagem[256];

        printf("Produtor (pai) iniciado. Digite mensagens (ou 'sair' para encerrar):\n");
        registrar_evento(registro, "Produtor iniciado.");

        while (1) {
            printf("> ");
            fflush(stdout); // esvazia o buffer pra garantir que o que esteja lá seja mostrado

            if (fgets(mensagem, sizeof(mensagem), stdin) == NULL) {
                break; // EOF (Ctrl+D)
            }

            // Remove o '\n' do final
            mensagem[strcspn(mensagem, "\n")] = '\0';

            // Se o usuário digitar "sair", encerra
            if (strcmp(mensagem, "sair") == 0) {
                break;
            }

            // Envia a mensagem pelo pipe
            write(pipeline[1], mensagem, strlen(mensagem));

            // Log da mensagem enviada
            char logmsg[300];
            snprintf(logmsg, sizeof(logmsg), "Mensagem enviada: %s", mensagem);
            registrar_evento(registro, logmsg);
        }

        close(pipeline[1]); // Fecha a escrita -> filho detecta EOF
        wait(NULL);        // Espera o filho terminar

        printf("Produtor (pai) encerrado.\n");
        registrar_evento(registro, "Produtor encerrado.");
    }

    return 0;
}