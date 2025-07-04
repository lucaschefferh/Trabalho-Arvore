// enumeração que define os tipos de nós na árvore
typedef enum{
    PASTA,    //representa um diretório
    ARQUIVO   // representa um arquivo
}tipoNo;

// número máximo de comandos armazenados no histórico
#define MAX_HISTORY 100

// tamanho máximo de cada comando no histórico
#define MAX_COMMAND_LENGTH 256

// cores ANSI para formatação do terminal
#define RESET_COLOR "\033[0m"        // reseta a cor para o padrão
#define GREEN_COLOR "\033[32m"       // cor verde normal
#define BOLD_GREEN_COLOR "\033[1;32m" // cor verde em negrito

// estrutura para armazenar o histórico de comandos do usuário
typedef struct {
    char comandos[MAX_HISTORY][MAX_COMMAND_LENGTH]; // array circular de comandos
    int total;   // número total de comandos armazenados
    int inicio;  // índice do primeiro comando no buffer circular
} HistoricoComandos;

// estrutura que representa um nó na árvore de diretórios
typedef struct no{
    char* nome;                  // nome do arquivo ou pasta
    tipoNo tipo;                 // tipo do nó (PASTA ou ARQUIVO)
    struct no* primeiroFilho;    // ponteiro para o primeiro filho
    struct no* proxIrmao;        // ponteiro para o próximo irmão
    struct no* pai;              // ponteiro para o nó pai
}no;

typedef no* Arvore;

// Comandos obrigatórios
void comando_ls(no* diretorio_atual);
void comando_cd(char* caminho, no** diretorio_atual, no* raiz);
void comando_search(char* nome, no* raiz);
void comando_rm(char* arquivo, no* diretorio_atual);
void comando_mkdir(char* pasta, no* diretorio_atual);
void comando_clear();
void comando_help();
void mostrar_alternativas(no* diretorio_atual, char* nome_parcial);

// Comandos extras
void comando_touch(char* arquivo, no* diretorio_atual); // cria um novo arquivo no diretório atual
void comando_tree(no* diretorio_atual); // exibe a estrutura da árvore a partir do diretório atual
void comando_pwd(no* diretorio_atual); // mostra o caminho completo do diretório atual
void comando_rmdir(char* pasta, no* diretorio_atual); // remove uma pasta (e todo seu conteúdo) do diretório atual
void comando_history(char* arg, HistoricoComandos* historico); // mostra o histórico de comandos executados

// Funçoes para construir o programa
no* criar_no(const char* nome, tipoNo tipo, no* pai); // função para criar um novo nó na árvore
void processar_caminho(no *raiz, char *caminho); // processa um caminho de arquivo/pasta e adiciona à árvore
void imprimir_arvore(no *raiz, int nivel); // imprime a estrutura da árvore de forma hierárquica
char* ler_arquivo(const char *arquivo); // lê o conteúdo de um arquivo e retorna como string
no* construir_arvore_do_arquivo(const char* arquivo); // constrói a árvore de diretórios a partir de um arquivo de entrada
void salvar_arvore_no_arquivo(no* raiz, const char* arquivo); // salva a estrutura da árvore em um arquivo
void escrever_caminhos_recursivo(no* no_atual, char* caminho_atual, FILE* arquivo); // função auxiliar para escrever caminhos recursivamente no arquivo
void iniciar_terminal(no* raiz, const char* arquivo_entrada); // inicia o simulador de terminal interativo
void processar_comando(char* comando, no** diretorio_atual, no* raiz, HistoricoComandos* historico); // processa e executa comandos digitados pelo usuário
void adicionar_ao_historico(HistoricoComandos* historico, const char* comando); // adiciona um comando ao histórico
void inicializar_historico(HistoricoComandos* historico); // inicializa a estrutura do histórico de comandos
char* obter_prompt_colorido(no* diretorio_atual); // retorna o prompt colorido com o caminho atual
no* encontrar_diretorio(no* base, char* nome); // encontra um diretório filho pelo nome
char* obter_caminho_completo(no* no_atual); // obtém o caminho completo de um nó
void buscar_recursivo(no* no_atual, char* nome_procurado, int* contador); // função auxiliar para busca recursiva na árvore
void liberar_no_recursivo(no* no_para_remover); // libera recursivamente a memória de um nó e seus filhos
int strcasecmp_custom(const char* s1, const char* s2); // compara duas strings ignorando maiúsculas/minúsculas
int strncasecmp_custom(const char* s1, const char* s2, size_t n); // compara n caracteres de duas strings ignorando maiúsculas/minúsculas
char* stristr_custom(const char* str, const char* substr); // busca uma substring em uma string ignorando maiúsculas/minúsculas
