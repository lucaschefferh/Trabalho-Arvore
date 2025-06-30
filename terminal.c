#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "terminal.h"


no* criar_no(const char* nome, tipoNo tipo, no* pai) {
    no* novo_no = (no*)malloc(sizeof(no));
    if (novo_no == NULL) {
        perror("Falha na alocação de memória para o nó");
        exit(EXIT_FAILURE);
    }
    
    novo_no->nome = strdup(nome); 
    novo_no->tipo = tipo;
    novo_no->pai = pai;
    novo_no->primeiroFilho = NULL;
    novo_no->proxIrmao = NULL;
    
    return novo_no;
}


char* ler_arquivo(const char *nome_ficheiro) {
    FILE *ficheiro = fopen(nome_ficheiro, "r");
    if (ficheiro == NULL) {
        perror("Erro ao abrir o ficheiro");
        return NULL;
    }

    // 1. Ir para o final do ficheiro para descobrir o seu tamanho
    fseek(ficheiro, 0, SEEK_END);
    long tamanho = ftell(ficheiro);
    // Voltar ao início do ficheiro para a leitura
    rewind(ficheiro); 

    // 2. Alocar memória para a string (+1 para o carácter nulo '\0')
    char *buffer = (char*) malloc(tamanho + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Falha ao alocar memória para ler o ficheiro.\n");
        fclose(ficheiro);
        return NULL;
    }

    // 3. Ler o conteúdo do ficheiro para o buffer
    size_t bytes_lidos = fread(buffer, 1, tamanho, ficheiro);
    if (bytes_lidos == 0 && tamanho > 0) {
        fprintf(stderr, "Erro ao ler o ficheiro.\n");
        free(buffer);
        fclose(ficheiro);
        return NULL;
    }

    // 4. Adicionar o terminador nulo para garantir que é uma string válida
    buffer[bytes_lidos] = '\0';

    fclose(ficheiro);
    return buffer;
}


void processar_caminho(no *raiz, char *caminho) {
    char *token = strtok(caminho, "/");
    no *no_atual = raiz;

    while (token!= NULL) {
        no *filho = no_atual->primeiroFilho;
        no *no_anterior = NULL;

        while (filho!= NULL) {
            if (strcmp(filho->nome, token) == 0) {
                break;
            }
            no_anterior = filho;
            filho = filho->proxIrmao;
        }

        if (filho!= NULL) {
            no_atual = filho;
        } else {
            // *** AQUI ESTÁ A MUDANÇA PRINCIPAL ***
            // Determina o tipo usando o enum com base na presença de um '.' [1]
            tipoNo tipo_do_no = (strchr(token, '.') != NULL)? ARQUIVO : PASTA;
            
            // Chama a função criar_no com o novo tipo
            no* novo_no = criar_no(token, tipo_do_no, no_atual);

            if (no_anterior == NULL) {
                no_atual->primeiroFilho = novo_no;
            } else {
                no_anterior->proxIrmao = novo_no;
            }
            no_atual = novo_no;
        }
        
        token = strtok(NULL, "/");
    }
}


// Função recursiva para imprimir a árvore e sua estrutura
void imprimir_arvore(no* no_atual, int nivel) {
    if (no_atual == NULL) {
        return;
    }

    // Imprime o recuo para mostrar a hierarquia
    for (int i = 0; i < nivel; i++) {
        printf("  ");
    }

    // Imprime o nome do nó e se é pasta ou arquivo
    printf("|-- %s (%s)\n", no_atual->nome, (no_atual->tipo == PASTA) ? "Pasta" : "Arquivo");

    // Chama recursivamente para todos os filhos
    no* filho = no_atual->primeiroFilho;
    while (filho != NULL) {
        imprimir_arvore(filho, nivel + 1);
        filho = filho->proxIrmao;
    }
}


// Função que lê um arquivo e constrói a árvore de diretórios
no* construir_arvore_do_arquivo(const char* nome_arquivo) {
    // Ler o conteúdo do arquivo
    char* str = ler_arquivo(nome_arquivo);
    if (str == NULL) {
        fprintf(stderr, "Erro ao ler o arquivo: %s\n", nome_arquivo);
        return NULL;
    }
    
    // Criar o nó raiz
    no* raiz = criar_no("/", PASTA, NULL);
    
    // Processar cada linha do arquivo manualmente
    char* inicio = str;
    char* fim = str;
    
    while (*fim != '\0') {
        // Encontrar o final da linha
        while (*fim != '\n' && *fim != '\0') {
            fim++;
        }
        
        // Se encontrou uma linha válida
        if (fim > inicio) {
            // Criar uma cópia da linha
            int tamanho = fim - inicio;
            char* linha = (char*)malloc(tamanho + 1);
            strncpy(linha, inicio, tamanho);
            linha[tamanho] = '\0';
            
            // Processar a linha
            processar_caminho(raiz, linha);
            free(linha);
        }
        
        // Avançar para a próxima linha
        if (*fim == '\n') {
            fim++;
        }
        inicio = fim;
    }
    
    // Liberar a memória da string lida
    free(str);
    
    // Retornar a árvore construída
    return raiz;
}


void iniciar_terminal(no* raiz) {
    no* diretorio_atual = raiz;
    char comando[256];
    
    printf("=== Simulador de Terminal ===\n");
    printf("Digite 'help' para ver os comandos disponiveis\n");
    printf("Digite 'exit' para sair\n\n");
    
    while (1) {
        // Mostrar prompt com caminho atual
        char* caminho = obter_caminho_completo(diretorio_atual);
        printf("%s$ ", caminho);
        free(caminho);
        
        // Ler comando do usuário
        if (fgets(comando, sizeof(comando), stdin) == NULL) {
            break;
        }
        
        // Remover quebra de linha
        comando[strcspn(comando, "\n")] = '\0';
        
        // Sair se comando for exit
        if (strcmp(comando, "exit") == 0) {
            printf("Saindo do terminal...\n");
            break;
        }
        
        // Processar comando
        processar_comando(comando, &diretorio_atual, raiz);
    }
}


void processar_comando(char* comando, no** diretorio_atual, no* raiz) {
    char* cmd = strtok(comando, " ");
    char* arg = strtok(NULL, ""); // Pegar o resto da linha inteira
    
    if (cmd == NULL) {
        return; // Comando vazio
    }
    
    if (strcmp(cmd, "ls") == 0) {
        comando_ls(*diretorio_atual);
    } else if (strcmp(cmd, "cd") == 0) {
        comando_cd(arg, diretorio_atual, raiz);
    } else if (strcmp(cmd, "pwd") == 0) {
        comando_pwd(*diretorio_atual);
    } else if (strcmp(cmd, "tree") == 0) {
        comando_tree(*diretorio_atual);
    } else if (strcmp(cmd, "help") == 0) {
        comando_help();
    } else if (strcmp(cmd, "search") == 0) {
        comando_search(arg, raiz);
    } else if (strcmp(cmd, "rm") == 0) {
        comando_rm(arg, *diretorio_atual);
    } else if (strcmp(cmd, "mkdir") == 0) {
        comando_mkdir(arg, *diretorio_atual);
    } else if (strcmp(cmd, "clear") == 0) {
        comando_clear();
    } else {
        printf("Comando '%s' nao reconhecido. Digite 'help' para ver comandos disponiveis.\n", cmd);
    }
}


void comando_ls(no* diretorio_atual) {
    no* filho = diretorio_atual->primeiroFilho;
    
    if (filho == NULL) {
        printf("Diretorio vazio\n");
        return;
    }
    
    while (filho != NULL) {
        if (filho->tipo == PASTA) {
            printf("[DIR]  %s/\n", filho->nome);
        } else {
            printf("[FILE] %s\n", filho->nome);
        }
        filho = filho->proxIrmao;
    }
}


void comando_cd(char* caminho, no** diretorio_atual, no* raiz) {
    if (caminho == NULL) {
        *diretorio_atual = raiz; // cd sem argumentos vai para raiz
        return;
    }
    
    if (strcmp(caminho, "/") == 0) {
        *diretorio_atual = raiz;
        return;
    }
    
    if (strcmp(caminho, "..") == 0) {
        if ((*diretorio_atual)->pai != NULL) {
            *diretorio_atual = (*diretorio_atual)->pai;
        }
        return;
    }
    
    // Procurar diretório filho
    no* destino = encontrar_diretorio(*diretorio_atual, caminho);
    if (destino != NULL && destino->tipo == PASTA) {
        *diretorio_atual = destino;
    } else {
        mostrar_alternativas(*diretorio_atual, caminho);
    }
}


void comando_pwd(no* diretorio_atual) {
    char* caminho = obter_caminho_completo(diretorio_atual);
    printf("%s\n", caminho);
    free(caminho);
}


void comando_tree(no* diretorio_atual) {
    printf("Estrutura a partir de %s:\n", diretorio_atual->nome);
    imprimir_arvore(diretorio_atual, 0);
}


void comando_search(char* nome, no* raiz) {
    if (nome == NULL) {
        printf("Uso: search <nome>\n");
        return;
    }
    
    int encontrados = 0;
    printf("Buscando por '%s'...\n", nome);
    buscar_recursivo(raiz, nome, &encontrados);
    
    if (encontrados == 0) {
        printf("Nenhum arquivo ou diretorio encontrado com o nome '%s'\n", nome);
    } else {
        printf("\nTotal encontrado: %d item(s)\n", encontrados);
    }
}


void buscar_recursivo(no* no_atual, char* nome_procurado, int* contador) {
    if (no_atual == NULL) {
        return;
    }
    
    // Verificar se o nome atual contém o texto procurado (busca parcial case-insensitive)
    if (stristr_custom(no_atual->nome, nome_procurado) != NULL) {
        char* caminho = obter_caminho_completo(no_atual);
        printf("[%s] %s\n", 
               (no_atual->tipo == PASTA) ? "DIR" : "FILE", 
               caminho);
        free(caminho);
        (*contador)++;
    }
    
    // Buscar recursivamente nos filhos
    no* filho = no_atual->primeiroFilho;
    while (filho != NULL) {
        buscar_recursivo(filho, nome_procurado, contador);
        filho = filho->proxIrmao;
    }
}


void comando_help() {
    printf("Comandos disponiveis:\n");
    printf("  ls           - Listar conteudo do diretorio atual\n");
    printf("  cd <dir>     - Mudar para diretorio especificado\n");
    printf("  cd ..        - Voltar para diretorio pai\n");
    printf("  cd /         - Ir para diretorio raiz\n");
    printf("  pwd          - Mostrar caminho atual\n");
    printf("  tree         - Mostrar arvore de diretorios\n");
    printf("  search <nome> - Buscar arquivo ou pasta pelo nome\n");
    printf("  mkdir <pasta> - Criar nova pasta no diretorio atual\n");
    printf("  rm <pasta>   - Remover pasta (somente pastas vazias ou com conteudo)\n");
    printf("  clear        - Limpar a tela do terminal\n");
    printf("  help         - Mostrar esta ajuda\n");
    printf("  exit         - Sair do terminal\n");
}


no* encontrar_diretorio(no* base, char* nome) {
    no* filho = base->primeiroFilho;
    
    while (filho != NULL) {
        if (strcasecmp_custom(filho->nome, nome) == 0) {
            return filho;
        }
        filho = filho->proxIrmao;
    }
    
    return NULL;
}


char* obter_caminho_completo(no* no_atual) {
    if (no_atual == NULL || no_atual->pai == NULL) {
        return strdup("/");
    }
    
    // Calcular tamanho necessário
    int tamanho = 0;
    no* temp = no_atual;
    while (temp != NULL && temp->pai != NULL) {
        tamanho += strlen(temp->nome) + 1; // +1 para '/'
        temp = temp->pai;
    }
    tamanho += 1; // Para o '/' inicial
    tamanho += 1; // Para '\0'
    
    // Construir caminho
    char* caminho = (char*)malloc(tamanho);
    strcpy(caminho, "/");
    
    // Usar recursão para construir o caminho
    char* caminho_pai = NULL;
    if (no_atual->pai->pai != NULL) {
        caminho_pai = obter_caminho_completo(no_atual->pai);
        strcpy(caminho, caminho_pai);
        free(caminho_pai);
    }
    
    if (strcmp(caminho, "/") != 0) {
        strcat(caminho, "/");
    }
    strcat(caminho, no_atual->nome);
    
    return caminho;
}


void mostrar_alternativas(no* diretorio_atual, char* nome_parcial) {
    no* filho = diretorio_atual->primeiroFilho;
    int encontrou_alternativas = 0;
    
    printf("Diretorio nao encontrado. Voce quis dizer:\n");
    
    // Procurar por diretórios que começam com o nome parcial (case-insensitive)
    while (filho != NULL) {
        if (filho->tipo == PASTA && strncasecmp_custom(filho->nome, nome_parcial, strlen(nome_parcial)) == 0) {
            printf("  %s/\n", filho->nome);
            encontrou_alternativas = 1;
        }
        filho = filho->proxIrmao;
    }
    
    if (!encontrou_alternativas) {
        printf("Diretorio nao encontrado\n");
    }
}


int strcasecmp_custom(const char* s1, const char* s2) {
    while (*s1 && *s2) {
        char c1 = (*s1 >= 'A' && *s1 <= 'Z') ? *s1 + 32 : *s1;
        char c2 = (*s2 >= 'A' && *s2 <= 'Z') ? *s2 + 32 : *s2;
        
        if (c1 != c2) {
            return c1 - c2;
        }
        s1++;
        s2++;
    }
    
    char c1 = (*s1 >= 'A' && *s1 <= 'Z') ? *s1 + 32 : *s1;
    char c2 = (*s2 >= 'A' && *s2 <= 'Z') ? *s2 + 32 : *s2;
    
    return c1 - c2;
}

// Funcao auxiliar para comparacao case-insensitive com limite (substitui strncasecmp)
int strncasecmp_custom(const char* s1, const char* s2, size_t n) {
    while (n > 0 && *s1 && *s2) {
        char c1 = (*s1 >= 'A' && *s1 <= 'Z') ? *s1 + 32 : *s1;
        char c2 = (*s2 >= 'A' && *s2 <= 'Z') ? *s2 + 32 : *s2;
        
        if (c1 != c2) {
            return c1 - c2;
        }
        s1++;
        s2++;
        n--;
    }
    
    if (n == 0) {
        return 0;
    }
    
    char c1 = (*s1 >= 'A' && *s1 <= 'Z') ? *s1 + 32 : *s1;
    char c2 = (*s2 >= 'A' && *s2 <= 'Z') ? *s2 + 32 : *s2;
    
    return c1 - c2;
}


// Funcao auxiliar para buscar substring case-insensitive (substitui strstr)
char* stristr_custom(const char* str, const char* substr) {
    if (!substr || !*substr) {
        return (char*)str;
    }
    
    while (*str) {
        const char* s1 = str;
        const char* s2 = substr;
        
        // Comparar caractere por caractere (case-insensitive)
        while (*s1 && *s2) {
            char c1 = (*s1 >= 'A' && *s1 <= 'Z') ? *s1 + 32 : *s1;
            char c2 = (*s2 >= 'A' && *s2 <= 'Z') ? *s2 + 32 : *s2;
            
            if (c1 != c2) {
                break;
            }
            s1++;
            s2++;
        }
        
        // Se chegou ao final da substring, encontrou
        if (!*s2) {
            return (char*)str;
        }
        
        str++;
    }
    
    return NULL;
}

// Funcao para liberar memoria de um no e todos os seus filhos recursivamente
void liberar_no_recursivo(no* no_para_remover) {
    if (no_para_remover == NULL) {
        return;
    }
    
    // Primeiro, liberar todos os filhos recursivamente
    no* filho = no_para_remover->primeiroFilho;
    while (filho != NULL) {
        no* proximo_filho = filho->proxIrmao;
        liberar_no_recursivo(filho);
        filho = proximo_filho;
    }
    
    // Liberar o nome e o proprio no
    if (no_para_remover->nome) {
        free(no_para_remover->nome);
    }
    free(no_para_remover);
}

// Comando rm - removes a directory and all its contents recursively
void comando_rm(char* nome_pasta, no* diretorio_atual) {
    if (nome_pasta == NULL || strlen(nome_pasta) == 0) {
        printf("Erro: Especifique o nome da pasta a ser removida.\n");
        printf("Uso: rm <nome_da_pasta>\n");
        return;
    }
    
    // Remover espacos em branco no inicio e fim
    while (*nome_pasta == ' ') nome_pasta++;
    char* fim = nome_pasta + strlen(nome_pasta) - 1;
    while (fim > nome_pasta && *fim == ' ') {
        *fim = '\0';
        fim--;
    }
    
    if (strlen(nome_pasta) == 0) {
        printf("Erro: Nome da pasta nao pode estar vazio.\n");
        return;
    }
    
    // Nao permitir remover diretorio atual ou pai
    if (strcmp(nome_pasta, ".") == 0 || strcmp(nome_pasta, "..") == 0) {
        printf("Erro: Nao e possivel remover '.' ou '..'.\n");
        return;
    }
    
    // Procurar a pasta no diretorio atual
    no* filho = diretorio_atual->primeiroFilho;
    no* anterior = NULL;
    
    while (filho != NULL) {
        if (strcasecmp_custom(filho->nome, nome_pasta) == 0) {
            // Verificar se e uma pasta
            if (filho->tipo != PASTA) {
                printf("Erro: '%s' nao e uma pasta. O comando rm remove apenas pastas.\n", nome_pasta);
                return;
            }
            
            // Remover o no da lista de filhos
            if (anterior == NULL) {
                // E o primeiro filho
                diretorio_atual->primeiroFilho = filho->proxIrmao;
            } else {
                // Nao e o primeiro filho
                anterior->proxIrmao = filho->proxIrmao;
            }
            
            // Liberar memoria recursivamente
            liberar_no_recursivo(filho);
            
            printf("Pasta '%s' removida com sucesso.\n", nome_pasta);
            return;
        }
        
        anterior = filho;
        filho = filho->proxIrmao;
    }
    
    // Se chegou ate aqui, a pasta nao foi encontrada
    printf("Erro: Pasta '%s' nao encontrada no diretorio atual.\n", nome_pasta);
    
    // Mostrar sugestoes se houver pastas com nomes similares
    printf("Pastas disponiveis:\n");
    filho = diretorio_atual->primeiroFilho;
    int encontrou_pastas = 0;
    while (filho != NULL) {
        if (filho->tipo == PASTA) {
            printf("  %s/\n", filho->nome);
            encontrou_pastas = 1;
        }
        filho = filho->proxIrmao;
    }
    
    if (!encontrou_pastas) {
        printf("  (nenhuma pasta encontrada)\n");
    }
}

// Comando mkdir - cria uma nova pasta no diretorio atual
void comando_mkdir(char* nome_pasta, no* diretorio_atual) {
    if (nome_pasta == NULL || strlen(nome_pasta) == 0) {
        printf("Erro: Especifique o nome da pasta a ser criada.\n");
        printf("Uso: mkdir <nome_da_pasta>\n");
        return;
    }
    
    // Remover espacos em branco no inicio e fim
    while (*nome_pasta == ' ') nome_pasta++;
    char* fim = nome_pasta + strlen(nome_pasta) - 1;
    while (fim > nome_pasta && *fim == ' ') {
        *fim = '\0';
        fim--;
    }
    
    if (strlen(nome_pasta) == 0) {
        printf("Erro: Nome da pasta nao pode estar vazio.\n");
        return;
    }
    
    // Verificar se o nome contem caracteres invalidos
    for (int i = 0; nome_pasta[i] != '\0'; i++) {
        if (nome_pasta[i] == '/' || nome_pasta[i] == '\\' || nome_pasta[i] == ':' || 
            nome_pasta[i] == '*' || nome_pasta[i] == '?' || nome_pasta[i] == '"' || 
            nome_pasta[i] == '<' || nome_pasta[i] == '>' || nome_pasta[i] == '|') {
            printf("Erro: Nome da pasta contem caracteres invalidos.\n");
            printf("Caracteres nao permitidos: / \\ : * ? \" < > |\n");
            return;
        }
    }
    
    // Nao permitir nomes especiais
    if (strcmp(nome_pasta, ".") == 0 || strcmp(nome_pasta, "..") == 0) {
        printf("Erro: Nao e possivel criar pasta com nome '.' ou '..'.\n");
        return;
    }
    
    // Verificar se ja existe uma pasta ou arquivo com o mesmo nome
    no* filho = diretorio_atual->primeiroFilho;
    while (filho != NULL) {
        if (strcasecmp_custom(filho->nome, nome_pasta) == 0) {
            if (filho->tipo == PASTA) {
                printf("Erro: Ja existe uma pasta com o nome '%s'.\n", nome_pasta);
            } else {
                printf("Erro: Ja existe um arquivo com o nome '%s'.\n", nome_pasta);
            }
            return;
        }
        filho = filho->proxIrmao;
    }
    
    // Criar o novo no (pasta)
    no* nova_pasta = criar_no(nome_pasta, PASTA, diretorio_atual);
    if (nova_pasta == NULL) {
        printf("Erro: Falha ao criar a pasta '%s'.\n", nome_pasta);
        return;
    }
    
    // Adicionar a nova pasta como filho do diretorio atual
    if (diretorio_atual->primeiroFilho == NULL) {
        // E o primeiro filho
        diretorio_atual->primeiroFilho = nova_pasta;
    } else {
        // Adicionar no final da lista de irmaos
        no* ultimo_filho = diretorio_atual->primeiroFilho;
        while (ultimo_filho->proxIrmao != NULL) {
            ultimo_filho = ultimo_filho->proxIrmao;
        }
        ultimo_filho->proxIrmao = nova_pasta;
    }
    
    printf("Pasta '%s' criada com sucesso.\n", nome_pasta);
}

// Comando clear - limpa a tela do terminal
void comando_clear() {
    printf("\033[2J\033[H");
}