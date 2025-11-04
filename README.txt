Pipeline Simples – Explicação detalhada

Começamos incluindo as bibliotecas necessárias, cada uma fornecendo funções específicas:

- stdio.h -> funções de entrada e saída: printf, fprintf, fopen, fclose, fgets.
- stdlib.h -> funções de utilidade geral: exit, constantes EXIT_FAILURE e EXIT_SUCCESS.
- unistd.h -> funções de baixo nível para manipulação de processos e pipes: pipe, fork, read, write, close.
- string.h -> funções de manipulação de strings: strlen, strcmp, strcspn, snprintf.
- sys/wait.h -> funções para sincronização de processos: wait.
- time.h -> funções para manipulação de tempo: time, ctime.

Função registrar_evento

Essa função é responsável por registrar todos os eventos do programa em um arquivo .txt.

- Abre o arquivo em modo append ("a"), garantindo que o histórico anterior não seja sobrescrito.
- Adiciona um timestamp usando time e ctime, armazenando o tempo atual na variável 'agora'. O formato usado é: dia_da_semana mês dia hora:minuto:segundo ano.
- Escreve a mensagem no arquivo e fecha o arquivo, garantindo que todas as interações fiquem registradas.

Função main

1. Criação do pipe

int pipeline[2];

- Um array de 2 inteiros:
  - pipeline[0] -> extremidade de leitura
  - pipeline[1] -> extremidade de escrita

pipe(pipeline);

- Cria um canal de comunicação unidirecional entre processos.
- Retorna -1 em caso de erro.

2. Criação do processo filho

pid = fork();

- fork() cria um novo processo.
- O retorno indica:
  - 0 -> estamos no filho
  - >0 -> estamos no pai (retorna o PID do filho)
  - <0 -> erro na criação do processo

3. Processo filho -> Consumidor

- Fecha a extremidade de escrita do pipe (close(pipeline[1])), pois ele só vai ler.
- Lê mensagens do pipe com read.
- Cada mensagem lida é:
  - exibida na tela com printf
  - registrada no arquivo de log com registrar_evento.
- O loop termina quando o pipe é fechado pelo pai (detecta EOF).
- Fecha a extremidade de leitura (close(pipeline[0])) e registra encerramento.

4. Processo pai -> Produtor

- Fecha a extremidade de leitura do pipe (close(pipeline[0])), pois ele só vai escrever.
- Lê mensagens do usuário (fgets) e envia pelo pipe com write.
- Cada mensagem enviada também é registrada no log.
- Quando o usuário digita "sair", o loop termina, fecha o pipe de escrita e espera o filho terminar (wait).
- Registra o encerramento no log.

Detalhes importantes

- fflush(stdout) -> garante que o prompt (>) apareça na tela imediatamente, mesmo com bufferização do stdout.
- Buffer do pipe -> permite que o pai escreva mais rápido que o filho lê, sem perder mensagens.
- const char *logfile -> protege a string literal de alterações e indica que o nome do arquivo não vai mudar.
