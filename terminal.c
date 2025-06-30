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
    fseek(ficheiro, 0, SEEK_END);
    long tamanho = ftell(ficheiro);
    rewind(ficheiro); 
    char *buffer = (char*) malloc(tamanho + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Falha ao alocar memória para ler o ficheiro.\n");
        fclose(ficheiro);
        return NULL;
    }
    size_t bytes_lidos = fread(buffer, 1, tamanho, ficheiro);
    if (bytes_lidos == 0 && tamanho > 0) {
        fprintf(stderr, "Erro ao ler o ficheiro.\n");
        free(buffer);
        fclose(ficheiro);
        return NULL;
    }
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
            tipoNo tipo_do_no = (strchr(token, '.') != NULL)? ARQUIVO : PASTA;
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
void imprimir_arvore(no* no_atual, int nivel) {
    if (no_atual == NULL) {
        return;
    }
    for (int i = 0; i < nivel; i++) {
        printf("  ");
    }
    printf("|-- %s (%s)\n", no_atual->nome, (no_atual->tipo == PASTA) ? "Pasta" : "Arquivo");
    no* filho = no_atual->primeiroFilho;
    while (filho != NULL) {
        imprimir_arvore(filho, nivel + 1);
        filho = filho->proxIrmao;
    }
}
no* construir_arvore_do_arquivo(const char* nome_arquivo) {
    char* str = ler_arquivo(nome_arquivo);
    if (str == NULL) {
        fprintf(stderr, "Erro ao ler o arquivo: %s\n", nome_arquivo);
        return NULL;
    }
    no* raiz = criar_no("/", PASTA, NULL);
    char* inicio = str;
    char* fim = str;
    while (*fim != '\0') {
        while (*fim != '\n' && *fim != '\0') {
            fim++;
        }
        if (fim > inicio) {
            int tamanho = fim - inicio;
            char* linha = (char*)malloc(tamanho + 1);
            strncpy(linha, inicio, tamanho);
            linha[tamanho] = '\0';
            if (strlen(linha) > 0) {
                int valido = 1;
                for (int i = 0; linha[i] != '\0'; i++) {
                    if (linha[i] < 32 || linha[i] > 126) {
                        valido = 0;
                        break;
                    }
                }
                if (valido) {
                    processar_caminho(raiz, linha);
                } else {
                    printf("Aviso: Linha corrompida ignorada no arquivo de entrada.\n");
                }
            }
            free(linha);
        }
        if (*fim == '\n') {
            fim++;
        }
        inicio = fim;
    }
    free(str);
    return raiz;
}
void iniciar_terminal(no* raiz, const char* arquivo_entrada) {
    no* diretorio_atual = raiz;
    char comando[256];
    HistoricoComandos historico;
    inicializar_historico(&historico);
    
    // Limpar a tela ao iniciar o terminal para simular entrada em shell
    comando_clear();
    
    printf("=== Simulador de Terminal ===\n");
    printf("Digite 'help' para ver os comandos disponiveis\n");
    printf("Digite 'exit' para sair\n\n");
    
    while (1) {
        char* prompt = obter_prompt_colorido(diretorio_atual);
        printf("%s", prompt);
        free(prompt);
        
        if (fgets(comando, sizeof(comando), stdin) == NULL) {
            break;
        }
        
        comando[strcspn(comando, "\n")] = '\0';
        
        if (strcmp(comando, "exit") == 0) {
            printf("Saindo do terminal...\n");
            printf("Salvando alteracoes no arquivo '%s'...\n", arquivo_entrada);
            salvar_arvore_no_arquivo(raiz, arquivo_entrada);
            printf("Alteracoes salvas com sucesso!\n");
            break;
        }
        
        if (strlen(comando) > 0) {
            adicionar_ao_historico(&historico, comando);
        }
        
        processar_comando(comando, &diretorio_atual, raiz, &historico);
    }
}
void processar_comando(char* comando, no** diretorio_atual, no* raiz, HistoricoComandos* historico) {
    char* cmd = strtok(comando, " ");
    char* arg = strtok(NULL, "");
    if (cmd == NULL) {
        return;
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
    } else if (strcmp(cmd, "rmdir") == 0) {
        comando_rmdir(arg, *diretorio_atual);
    } else if (strcmp(cmd, "mkdir") == 0) {
        comando_mkdir(arg, *diretorio_atual);
    } else if (strcmp(cmd, "touch") == 0) {
        comando_touch(arg, *diretorio_atual);
    } else if (strcmp(cmd, "clear") == 0) {
        comando_clear();
    } else if (strcmp(cmd, "history") == 0) {
        comando_history(arg, historico);
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
        *diretorio_atual = raiz; 
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
    if (stristr_custom(no_atual->nome, nome_procurado) != NULL) {
        char* caminho = obter_caminho_completo(no_atual);
        printf("[%s] %s\n", 
               (no_atual->tipo == PASTA) ? "DIR" : "FILE", 
               caminho);
        free(caminho);
        (*contador)++;
    }
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
    printf("  touch <arquivo> - Criar novo arquivo (ex: touch arquivo.txt)\n");
    printf("  rm <arquivo> - Remover arquivo (somente arquivos)\n");
    printf("  rmdir <pasta> - Remover pasta (somente pastas)\n");
    printf("  clear        - Limpar a tela do terminal\n");
    printf("  history [n]  - Mostrar historico de comandos (padrao: 5)\n");
    printf("  help         - Mostrar esta ajuda\n");
    printf("  exit         - Sair do terminal (salva alteracoes automaticamente)\n");
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
    int tamanho = 0;
    no* temp = no_atual;
    while (temp != NULL && temp->pai != NULL) {
        tamanho += strlen(temp->nome) + 1; 
        temp = temp->pai;
    }
    tamanho += 1; 
    tamanho += 1; 
    char* caminho = (char*)malloc(tamanho);
    strcpy(caminho, "/");
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
char* stristr_custom(const char* str, const char* substr) {
    if (!substr || !*substr) {
        return (char*)str;
    }
    while (*str) {
        const char* s1 = str;
        const char* s2 = substr;
        while (*s1 && *s2) {
            char c1 = (*s1 >= 'A' && *s1 <= 'Z') ? *s1 + 32 : *s1;
            char c2 = (*s2 >= 'A' && *s2 <= 'Z') ? *s2 + 32 : *s2;
            if (c1 != c2) {
                break;
            }
            s1++;
            s2++;
        }
        if (!*s2) {
            return (char*)str;
        }
        str++;
    }
    return NULL;
}
void liberar_no_recursivo(no* no_para_remover) {
    if (no_para_remover == NULL) {
        return;
    }
    no* filho = no_para_remover->primeiroFilho;
    while (filho != NULL) {
        no* proximo_filho = filho->proxIrmao;
        liberar_no_recursivo(filho);
        filho = proximo_filho;
    }
    if (no_para_remover->nome) {
        free(no_para_remover->nome);
    }
    free(no_para_remover);
}
void comando_rm(char* nome_arquivo, no* diretorio_atual) {
    if (nome_arquivo == NULL || strlen(nome_arquivo) == 0) {
        printf("Erro: Especifique o nome do arquivo a ser removido.\n");
        printf("Uso: rm <nome_do_arquivo>\n");
        return;
    }
    while (*nome_arquivo == ' ') nome_arquivo++;
    char* fim = nome_arquivo + strlen(nome_arquivo) - 1;
    while (fim > nome_arquivo && *fim == ' ') {
        *fim = '\0';
        fim--;
    }
    if (strlen(nome_arquivo) == 0) {
        printf("Erro: Nome do arquivo nao pode estar vazio.\n");
        return;
    }
    no* filho = diretorio_atual->primeiroFilho;
    no* anterior = NULL;
    while (filho != NULL) {
        if (strcasecmp_custom(filho->nome, nome_arquivo) == 0) {
            if (filho->tipo != ARQUIVO) {
                printf("Erro: '%s' nao e um arquivo. Use 'rmdir' para remover pastas.\n", nome_arquivo);
                return;
            }
            if (anterior == NULL) {
                diretorio_atual->primeiroFilho = filho->proxIrmao;
            } else {
                anterior->proxIrmao = filho->proxIrmao;
            }
            if (filho->nome) {
                free(filho->nome);
            }
            free(filho);
            printf("Arquivo '%s' removido com sucesso.\n", nome_arquivo);
            return;
        }
        anterior = filho;
        filho = filho->proxIrmao;
    }
    printf("Erro: Arquivo '%s' nao encontrado no diretorio atual.\n", nome_arquivo);
    printf("Arquivos disponiveis:\n");
    filho = diretorio_atual->primeiroFilho;
    int encontrou_arquivos = 0;
    while (filho != NULL) {
        if (filho->tipo == ARQUIVO) {
            printf("  %s\n", filho->nome);
            encontrou_arquivos = 1;
        }
        filho = filho->proxIrmao;
    }
    if (!encontrou_arquivos) {
        printf("  (nenhum arquivo encontrado)\n");
    }
}
void comando_mkdir(char* nome_pasta, no* diretorio_atual) {
    if (nome_pasta == NULL || strlen(nome_pasta) == 0) {
        printf("Erro: Especifique o nome da pasta a ser criada.\n");
        printf("Uso: mkdir <nome_da_pasta>\n");
        return;
    }
    char* nome_original = nome_pasta;
    int apenas_espacos_original = 1;
    for (int i = 0; nome_original[i] != '\0'; i++) {
        if (nome_original[i] != ' ' && nome_original[i] != '\t') {
            apenas_espacos_original = 0;
            break;
        }
    }
    if (apenas_espacos_original) {
        printf("Erro: Nome da pasta nao pode conter apenas espacos em branco.\n");
        return;
    }
    while (*nome_pasta == ' ' || *nome_pasta == '\t') nome_pasta++;
    char* fim = nome_pasta + strlen(nome_pasta) - 1;
    while (fim > nome_pasta && (*fim == ' ' || *fim == '\t')) {
        *fim = '\0';
        fim--;
    }
    if (strlen(nome_pasta) == 0) {
        printf("Erro: Nome da pasta nao pode estar vazio ou conter apenas espacos.\n");
        return;
    }
    for (int i = 0; nome_pasta[i] != '\0'; i++) {
        if (nome_pasta[i] == '/' || nome_pasta[i] == '\\' || nome_pasta[i] == ':' || 
            nome_pasta[i] == '*' || nome_pasta[i] == '?' || nome_pasta[i] == '"' || 
            nome_pasta[i] == '<' || nome_pasta[i] == '>' || nome_pasta[i] == '|') {
            printf("Erro: Nome da pasta contem caracteres invalidos.\n");
            printf("Caracteres nao permitidos: / \\ : * ? \" < > |\n");
            return;
        }
    }
    if (strcmp(nome_pasta, ".") == 0 || strcmp(nome_pasta, "..") == 0) {
        printf("Erro: Nao e possivel criar pasta com nome '.' ou '..'.\n");
        return;
    }
    int tem_extensao = 0;
    char* extensao_encontrada = NULL;
    for (int i = 0; nome_pasta[i] != '\0'; i++) {
        if (nome_pasta[i] == '.') {
            tem_extensao = 1;
            extensao_encontrada = &nome_pasta[i];
            break;
        }
    }
    if (tem_extensao) {
        printf("Aviso: O nome '%s' parece ser um arquivo (tem extensao %s).\n", nome_pasta, extensao_encontrada);
        printf("Para criar um arquivo, use: touch %s\n", nome_pasta);
        printf("Para criar uma pasta mesmo assim, digite 'y' e pressione Enter.\n");
        printf("Para cancelar, digite qualquer outra tecla e pressione Enter.\n");
        printf("Sua escolha: ");
        char resposta[10];
        if (fgets(resposta, sizeof(resposta), stdin) == NULL) {
            printf("Operacao cancelada.\n");
            return;
        }
        resposta[strcspn(resposta, "\n")] = '\0';
        if (strcmp(resposta, "y") != 0 && strcmp(resposta, "Y") != 0) {
            printf("Operacao cancelada. Use 'touch %s' para criar um arquivo.\n", nome_pasta);
            return;
        }
        printf("Criando pasta '%s' (mesmo com extensao)...\n", nome_pasta);
    }
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
    no* nova_pasta = criar_no(nome_pasta, PASTA, diretorio_atual);
    if (nova_pasta == NULL) {
        printf("Erro: Falha ao criar a pasta '%s'.\n", nome_pasta);
        return;
    }
    if (diretorio_atual->primeiroFilho == NULL) {
        diretorio_atual->primeiroFilho = nova_pasta;
    } else {
        no* ultimo_filho = diretorio_atual->primeiroFilho;
        while (ultimo_filho->proxIrmao != NULL) {
            ultimo_filho = ultimo_filho->proxIrmao;
        }
        ultimo_filho->proxIrmao = nova_pasta;
    }
    printf("Pasta '%s' criada com sucesso.\n", nome_pasta);
}
void comando_touch(char* nome_arquivo, no* diretorio_atual) {
    if (nome_arquivo == NULL || strlen(nome_arquivo) == 0) {
        printf("Erro: Especifique o nome do arquivo a ser criado.\n");
        printf("Uso: touch <nome_do_arquivo>\n");
        printf("Exemplos: touch arquivo.txt, touch script.py, touch documento.doc\n");
        return;
    }
    while (*nome_arquivo == ' ' || *nome_arquivo == '\t') nome_arquivo++;
    char* fim = nome_arquivo + strlen(nome_arquivo) - 1;
    while (fim > nome_arquivo && (*fim == ' ' || *fim == '\t')) {
        *fim = '\0';
        fim--;
    }
    if (strlen(nome_arquivo) == 0) {
        printf("Erro: Nome do arquivo nao pode estar vazio ou conter apenas espacos.\n");
        return;
    }
    int apenas_espacos = 1;
    for (int i = 0; nome_arquivo[i] != '\0'; i++) {
        if (nome_arquivo[i] != ' ' && nome_arquivo[i] != '\t') {
            apenas_espacos = 0;
            break;
        }
    }
    if (apenas_espacos) {
        printf("Erro: Nome do arquivo nao pode conter apenas espacos em branco.\n");
        return;
    }
    for (int i = 0; nome_arquivo[i] != '\0'; i++) {
        if (nome_arquivo[i] == '/' || nome_arquivo[i] == '\\' || nome_arquivo[i] == ':' || 
            nome_arquivo[i] == '*' || nome_arquivo[i] == '?' || nome_arquivo[i] == '"' || 
            nome_arquivo[i] == '<' || nome_arquivo[i] == '>' || nome_arquivo[i] == '|') {
            printf("Erro: Nome do arquivo contem caracteres invalidos.\n");
            printf("Caracteres nao permitidos: / \\ : * ? \" < > |\n");
            return;
        }
    }
    if (strcmp(nome_arquivo, ".") == 0 || strcmp(nome_arquivo, "..") == 0) {
        printf("Erro: Nao e possivel criar arquivo com nome '.' ou '..'.\n");
        return;
    }
    no* filho = diretorio_atual->primeiroFilho;
    while (filho != NULL) {
        if (strcasecmp_custom(filho->nome, nome_arquivo) == 0) {
            if (filho->tipo == ARQUIVO) {
                printf("Erro: Ja existe um arquivo com o nome '%s'.\n", nome_arquivo);
            } else {
                printf("Erro: Ja existe uma pasta com o nome '%s'.\n", nome_arquivo);
            }
            return;
        }
        filho = filho->proxIrmao;
    }
    int tem_extensao = 0;
    for (int i = 0; nome_arquivo[i] != '\0'; i++) {
        if (nome_arquivo[i] == '.') {
            tem_extensao = 1;
            break;
        }
    }
    if (!tem_extensao) {
        printf("Aviso: Arquivo '%s' sera criado sem extensao.\n", nome_arquivo);
        printf("Sugestao: Use uma extensao como .txt, .c, .py, .doc, etc.\n");
    }
    no* novo_arquivo = criar_no(nome_arquivo, ARQUIVO, diretorio_atual);
    if (novo_arquivo == NULL) {
        printf("Erro: Falha ao criar o arquivo '%s'.\n", nome_arquivo);
        return;
    }
    if (diretorio_atual->primeiroFilho == NULL) {
        diretorio_atual->primeiroFilho = novo_arquivo;
    } else {
        no* ultimo_filho = diretorio_atual->primeiroFilho;
        while (ultimo_filho->proxIrmao != NULL) {
            ultimo_filho = ultimo_filho->proxIrmao;
        }
        ultimo_filho->proxIrmao = novo_arquivo;
    }
    if (tem_extensao) {
        char* extensao = strrchr(nome_arquivo, '.');
        printf("Arquivo '%s' criado com sucesso (tipo: %s).\n", nome_arquivo, extensao);
    } else {
        printf("Arquivo '%s' criado com sucesso (sem extensao).\n", nome_arquivo);
    }
}
void comando_rmdir(char* nome_pasta, no* diretorio_atual) {
    if (nome_pasta == NULL || strlen(nome_pasta) == 0) {
        printf("Erro: Especifique o nome da pasta a ser removida.\n");
        printf("Uso: rmdir <nome_da_pasta>\n");
        return;
    }
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
    if (strcmp(nome_pasta, ".") == 0 || strcmp(nome_pasta, "..") == 0) {
        printf("Erro: Nao e possivel remover '.' ou '..'.\n");
        return;
    }
    no* filho = diretorio_atual->primeiroFilho;
    no* anterior = NULL;
    while (filho != NULL) {
        if (strcasecmp_custom(filho->nome, nome_pasta) == 0) {
            if (filho->tipo != PASTA) {
                printf("Erro: '%s' nao e uma pasta. Use 'rm' para remover arquivos.\n", nome_pasta);
                return;
            }
            if (anterior == NULL) {
                diretorio_atual->primeiroFilho = filho->proxIrmao;
            } else {
                anterior->proxIrmao = filho->proxIrmao;
            }
            liberar_no_recursivo(filho);
            printf("Pasta '%s' removida com sucesso.\n", nome_pasta);
            return;
        }
        anterior = filho;
        filho = filho->proxIrmao;
    }
    printf("Erro: Pasta '%s' nao encontrada no diretorio atual.\n", nome_pasta);
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
void comando_clear() {
    printf("\033[2J\033[H");
}
void escrever_caminhos_recursivo(no* no_atual, char* caminho_atual, FILE* arquivo) {
    if (no_atual == NULL) {
        return;
    }
    char* caminho_completo;
    if (strcmp(caminho_atual, "/") == 0) {
        caminho_completo = malloc(strlen(caminho_atual) + strlen(no_atual->nome) + 1);
        sprintf(caminho_completo, "%s%s", caminho_atual, no_atual->nome);
    } else {
        caminho_completo = malloc(strlen(caminho_atual) + strlen(no_atual->nome) + 2);
        sprintf(caminho_completo, "%s/%s", caminho_atual, no_atual->nome);
    }
    if (strcmp(no_atual->nome, "/") != 0) {
        fprintf(arquivo, "%s\n", caminho_completo);
    }
    no* filho = no_atual->primeiroFilho;
    while (filho != NULL) {
        if (strcmp(no_atual->nome, "/") == 0) {
            escrever_caminhos_recursivo(filho, "", arquivo);
        } else {
            escrever_caminhos_recursivo(filho, caminho_completo, arquivo);
        }
        filho = filho->proxIrmao;
    }
    free(caminho_completo);
}
void salvar_arvore_no_arquivo(no* raiz, const char* nome_arquivo) {
    FILE* arquivo = fopen(nome_arquivo, "w");
    if (arquivo == NULL) {
        printf("Erro: Não foi possível abrir o arquivo '%s' para escrita.\n", nome_arquivo);
        return;
    }
    escrever_caminhos_recursivo(raiz, "/", arquivo);
    fclose(arquivo);
}

void inicializar_historico(HistoricoComandos* historico) {
    historico->total = 0;
    historico->inicio = 0;
}

void adicionar_ao_historico(HistoricoComandos* historico, const char* comando) {
    if (strlen(comando) == 0 || strcmp(comando, "history") == 0) {
        return;
    }
    
    int indice = (historico->inicio + historico->total) % MAX_HISTORY;
    strncpy(historico->comandos[indice], comando, MAX_COMMAND_LENGTH - 1);
    historico->comandos[indice][MAX_COMMAND_LENGTH - 1] = '\0';
    
    if (historico->total < MAX_HISTORY) {
        historico->total++;
    } else {
        historico->inicio = (historico->inicio + 1) % MAX_HISTORY;
    }
}

void comando_history(char* arg, HistoricoComandos* historico) {
    int num_mostrar = 5;
    
    if (arg != NULL && strlen(arg) > 0) {
        num_mostrar = atoi(arg);
        if (num_mostrar <= 0) {
            printf("Uso: history [numero]\n");
            printf("Exemplo: history 10 (mostra os ultimos 10 comandos)\n");
            return;
        }
    }
    
    if (historico->total == 0) {
        printf("Nenhum comando no historico.\n");
        return;
    }
    
    if (num_mostrar > historico->total) {
        num_mostrar = historico->total;
    }
    
    printf("Historico dos ultimos %d comandos:\n", num_mostrar);
    
    int inicio_mostrar = historico->total - num_mostrar;
    for (int i = 0; i < num_mostrar; i++) {
        int indice = (historico->inicio + inicio_mostrar + i) % MAX_HISTORY;
        printf("%3d  %s\n", inicio_mostrar + i + 1, historico->comandos[indice]);
    }
}

char* obter_prompt_colorido(no* diretorio_atual) {
    char* caminho_simples = obter_caminho_completo(diretorio_atual);
    
    size_t tamanho_necessario = strlen(caminho_simples) + strlen(BOLD_GREEN_COLOR) + strlen(RESET_COLOR) + 10;
    char* prompt_colorido = (char*)malloc(tamanho_necessario);
    
    if (prompt_colorido == NULL) {
        free(caminho_simples);
        return strdup("$");
    }
    
    sprintf(prompt_colorido, "%s%s%s$ ", BOLD_GREEN_COLOR, caminho_simples, RESET_COLOR);
    
    free(caminho_simples);
    return prompt_colorido;
}