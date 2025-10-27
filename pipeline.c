#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>

int main() {
    int pipe_io[2];  // pipefd[0] = leitura, pipefd[1] = escrita
    pid_t pid; // guarda o identificador (PID) so processo

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

        char buffer[100];
        ssize_t bytes_lidos;

        printf("Consumidor (filho) iniciado, aguardando mensagens...\n");

        // Lê continuamente até o pipe ser fechado pelo produtor
        while ((bytes_lidos = read(pipe_io[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes_lidos] = '\0'; // Garante fim de string
            printf("Hora recebida: %s\n", buffer);
            fflush(stdout);
        }

        close(pipe_io[0]); // Fecha a extremidade de leitura
        printf("Consumidor encerrado.\n");
        exit(EXIT_SUCCESS);
    }

    // Processo pai → PRODUTOR
    else {
        close(pipe_io[0]); // Fecha a extremidade de leitura

        for (int i = 0; i < 5; i++) { // envia 5 horários
            time_t t = time(NULL);
            struct tm *tm_info = localtime(&t);
            char hora[100];

            strftime(hora, sizeof(hora), "%H:%M:%S", tm_info);

            // Escreve no pipe
            write(pipe_io[1], hora, strlen(hora));

            printf("Produtor (pai) enviou: %s\n", hora);
            fflush(stdout);

            sleep(5); // Espera 5 segundos antes de enviar o próximo horário
        }

        close(pipe_io[1]); // Fecha a extremidade de escrita
        wait(NULL);       // Espera o processo filho terminar

        printf("Produtor (pai) encerrado.\n");
    }

    return 0;
}
