//Declaração de enum para identificar se é um arquivo ou uma pasta
typedef enum{
    PASTA,
    ARQUIVO
}tipoNo;

/*
Declaração da árvore para armazenar os dados, uma árvore que tem o ponteiro para o pai, para facilitar a navegação, e que tem uma lista encadeada como filhos, pois 
é permitido ter vários filhos para um só pai.      
*/

typedef struct no{
    char* nome;
    tipoNo tipo;
    struct no* primeiroFilho;
    struct no* proxIrmao;
    struct no* pai;
}no;

typedef no* Arvore;

// ===== FUNCOES DE GERENCIAMENTO DA ARVORE =====
no* criar_no(const char* nome, tipoNo tipo, no* pai);
void processar_caminho(no *raiz, char *caminho);
void imprimir_arvore(no *raiz, int nivel);

// ===== FUNCOES DE ARQUIVO =====
char* ler_arquivo(const char *nome_ficheiro);
no* construir_arvore_do_arquivo(const char* nome_arquivo);

// ===== FUNCOES DO SIMULADOR DE TERMINAL =====
void iniciar_terminal(no* raiz);
void processar_comando(char* comando, no** diretorio_atual, no* raiz);

// ===== COMANDOS DO TERMINAL =====
void comando_ls(no* diretorio_atual);
void comando_cd(char* caminho, no** diretorio_atual, no* raiz);
void comando_pwd(no* diretorio_atual);
void comando_tree(no* diretorio_atual);
void comando_search(char* nome, no* raiz);
void comando_rm(char* nome_pasta, no* diretorio_atual);
void comando_mkdir(char* nome_pasta, no* diretorio_atual);
void comando_clear();
void comando_help();

// ===== FUNCOES AUXILIARES =====
no* encontrar_diretorio(no* base, char* nome);
char* obter_caminho_completo(no* no_atual);
void mostrar_alternativas(no* diretorio_atual, char* nome_parcial);
void buscar_recursivo(no* no_atual, char* nome_procurado, int* contador);
void liberar_no_recursivo(no* no_para_remover);
int strcasecmp_custom(const char* s1, const char* s2);
int strncasecmp_custom(const char* s1, const char* s2, size_t n);
char* stristr_custom(const char* str, const char* substr);