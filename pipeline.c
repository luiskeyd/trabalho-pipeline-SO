#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main() {
    int pipe_io[2];  // pipe_io[0] = leitura, pipe_io[1] = escrita
    pid_t pid;

    // Cria o pipe
    if (pipe(pipe_io) == -1) {
        perror("Erro ao criar o pipe");
        exit(EXIT_FAILURE);
    }

    // Cria o processo filho
    pid = fork();

    if (pid < 0) {
        perror("Erro ao criar o processo");
        exit(EXIT_FAILURE);
    }

    // Processo filho → CONSUMIDOR
    if (pid == 0) {
        close(pipe_io[1]); // Fecha a extremidade de escrita

        char buffer[256];
        ssize_t bytes_lidos;

        printf("Consumidor (filho) iniciado, aguardando mensagens...\n");

        // Lê continuamente até o pipe ser fechado
        while ((bytes_lidos = read(pipe_io[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes_lidos] = '\0'; // Garante fim de string
            printf("Mensagem recebida: %s\n", buffer);
            fflush(stdout);
        }

        close(pipe_io[0]);
        printf("Consumidor encerrado.\n");
        exit(EXIT_SUCCESS);
    }

    // Processo pai → PRODUTOR
    else {
        close(pipe_io[0]); // Fecha a extremidade de leitura

        char mensagem[256];

        printf("Produtor (pai) iniciado. Digite mensagens (ou 'sair' para encerrar):\n");

        while (1) {
            printf("> ");
            fflush(stdout);

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
            write(pipe_io[1], mensagem, strlen(mensagem));
        }

        close(pipe_io[1]); // Fecha a escrita → filho detecta EOF
        wait(NULL);        // Espera o filho terminar
        printf("Produtor (pai) encerrado.\n");
    }

    return 0;
}
