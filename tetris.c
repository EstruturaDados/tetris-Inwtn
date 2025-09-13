#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TAM_FILA 5
#define TAM_PILHA 3
#define MAX_NOME 3

typedef struct {
    char nome[MAX_NOME]; // tipo da peça: "I", "O", "T", "L", etc
    int id;             // id único
} Peca;

typedef struct {
    Peca fila[TAM_FILA];
    int frente;
    int tras;
    int tamanho;
} FilaCircular;

typedef struct {
    Peca pilha[TAM_PILHA];
    int topo;
} Pilha;

typedef enum {
    JOGAR_PECA = 1,
    RESERVAR_PECA,
    USAR_PECA_RESERVADA,
    TROCAR_TOPO_PILHA_FILA,
    DESFAZER_ULTIMA_JOGADA,
    INVERTER_FILA_COM_PILHA,
    SAIR = 0
} OpcaoMenu;

typedef enum {
    ACAO_NENHUMA,
    ACAO_JOGAR,
    ACAO_RESERVAR,
    ACAO_USAR_RESERVADA,
    ACAO_TROCAR,
    ACAO_INVERTER
} TipoAcao;

typedef struct {
    TipoAcao acao;
    Peca pecaFilaAntes;   // peça da frente da fila antes da ação
    Peca pecaPilhaAntes;  // peça do topo da pilha antes da ação
    int pilhaTopoAntes;
    int filaFrenteAntes;
    int filaTrasAntes;
    int filaTamanhoAntes;
    int pilhaTopoIndexAntes;
    Peca pilhaAntes[TAM_PILHA];
    FilaCircular filaAntes;
    Pilha pilhaAntesEstado;
} EstadoJogo;

// Variáveis globais para controle
FilaCircular fila;
Pilha pilha;
int idGlobal = 1;
EstadoJogo estadoAnterior;
bool podeDesfazer = false;

// Protótipos
void inicializarFila(FilaCircular *f);
void inicializarPilha(Pilha *p);
Peca gerarPeca();
void enfileirar(FilaCircular *f, Peca p);
Peca desenfileirar(FilaCircular *f);
bool pilhaVazia(Pilha *p);
bool pilhaCheia(Pilha *p);
void empilhar(Pilha *p, Peca peca);
Peca desempilhar(Pilha *p);
void mostrarFila(FilaCircular *f);
void mostrarPilha(Pilha *p);
void salvarEstado();
void desfazer();
void jogarPeca();
void reservarPeca();
void usarPecaReservada();
void trocarTopoPilhaFila();
void inverterFilaComPilha();
void menu();

int main() {
    srand((unsigned)time(NULL));
    inicializarFila(&fila);
    inicializarPilha(&pilha);

    // Inicializa fila com 5 peças
    for (int i = 0; i < TAM_FILA; i++) {
        Peca p = gerarPeca();
        enfileirar(&fila, p);
    }

    printf("Bem-vindo ao Tetris Stack - Nivel Mestre!\n");
    menu();

    return 0;
}

// Inicializa fila vazia
void inicializarFila(FilaCircular *f) {
    f->frente = 0;
    f->tras = TAM_FILA - 1;
    f->tamanho = 0;
}

// Inicializa pilha vazia
void inicializarPilha(Pilha *p) {
    p->topo = -1;
}

// Gera peça aleatória com id único
Peca gerarPeca() {
    const char* tipos[] = {"I", "O", "T", "L", "J", "S", "Z"};
    int nTipos = sizeof(tipos) / sizeof(tipos[0]);
    Peca p;
    strcpy(p.nome, tipos[rand() % nTipos]);
    p.id = idGlobal++;
    return p;
}

// Enfileira peça na fila circular
void enfileirar(FilaCircular *f, Peca p) {
    if (f->tamanho == TAM_FILA) {
        printf("Fila cheia! Nao e possivel enfileirar.\n");
        return;
    }
    f->tras = (f->tras + 1) % TAM_FILA;
    f->fila[f->tras] = p;
    f->tamanho++;
}

// Desenfileira peça da frente da fila
Peca desenfileirar(FilaCircular *f) {
    Peca p = {"", -1};
    if (f->tamanho == 0) {
        printf("Fila vazia! Nao ha peça para jogar.\n");
        return p;
    }
    p = f->fila[f->frente];
    f->frente = (f->frente + 1) % TAM_FILA;
    f->tamanho--;
    return p;
}

// Verifica se pilha está vazia
bool pilhaVazia(Pilha *p) {
    return p->topo == -1;
}

// Verifica se pilha está cheia
bool pilhaCheia(Pilha *p) {
    return p->topo == TAM_PILHA - 1;
}

// Empilha peça na pilha
void empilhar(Pilha *p, Peca peca) {
    if (pilhaCheia(p)) {
        printf("Pilha cheia! Nao e possivel reservar mais pecas.\n");
        return;
    }
    p->pilha[++p->topo] = peca;
}

// Desempilha peça da pilha
Peca desempilhar(Pilha *p) {
    Peca peca = {"", -1};
    if (pilhaVazia(p)) {
        printf("Pilha vazia! Nao ha pecas reservadas para usar.\n");
        return peca;
    }
    return p->pilha[p->topo--];
}

// Mostra estado atual da fila
void mostrarFila(FilaCircular *f) {
    printf("\nFila atual (frente -> tras):\n");
    if (f->tamanho == 0) {
        printf("Fila vazia.\n");
        return;
    }
    int idx = f->frente;
    for (int i = 0; i < f->tamanho; i++) {
        Peca p = f->fila[idx];
        printf("  [%d] Tipo: %s | ID: %d\n", i+1, p.nome, p.id);
        idx = (idx + 1) % TAM_FILA;
    }
}

// Mostra estado atual da pilha
void mostrarPilha(Pilha *p) {
    printf("\nPilha de reserva (topo -> base):\n");
    if (pilhaVazia(p)) {
        printf("Pilha vazia.\n");
        return;
    }
    for (int i = p->topo; i >= 0; i--) {
        Peca peca = p->pilha[i];
        printf("  [%d] Tipo: %s | ID: %d\n", i+1, peca.nome, peca.id);
    }
}

// Salva estado atual para desfazer
void salvarEstado() {
    estadoAnterior.acao = ACAO_NENHUMA;
    estadoAnterior.filaAntes = fila;
    estadoAnterior.pilhaAntesEstado = pilha;
    podeDesfazer = true;
}

// Desfaz última jogada
void desfazer() {
    if (!podeDesfazer) {
        printf("Nao ha acao para desfazer.\n");
        return;
    }
    fila = estadoAnterior.filaAntes;
    pilha = estadoAnterior.pilhaAntesEstado;
    printf("Ultima acao desfeita com sucesso.\n");
    podeDesfazer = false;
}

// Jogar peça da frente da fila
void jogarPeca() {
    if (fila.tamanho == 0) {
        printf("Fila vazia! Nao ha peça para jogar.\n");
        return;
    }
    salvarEstado();
    Peca p = desenfileirar(&fila);
    printf("Jogou a peca: Tipo %s | ID %d\n", p.nome, p.id);
    Peca nova = gerarPeca();
    enfileirar(&fila, nova);
    printf("Nova peca gerada e adicionada na fila: Tipo %s | ID %d\n", nova.nome, nova.id);
    estadoAnterior.acao = ACAO_JOGAR;
}

// Reservar peça da frente da fila (empilhar)
void reservarPeca() {
    if (fila.tamanho == 0) {
        printf("Fila vazia! Nao ha peça para reservar.\n");
        return;
    }
    if (pilhaCheia(&pilha)) {
        printf("Pilha cheia! Nao e possivel reservar mais pecas.\n");
        return;
    }
    salvarEstado();
    Peca p = desenfileirar(&fila);
    empilhar(&pilha, p);
    printf("Peca reservada: Tipo %s | ID %d\n", p.nome, p.id);
    Peca nova = gerarPeca();
    enfileirar(&fila, nova);
    printf("Nova peca gerada e adicionada na fila: Tipo %s | ID %d\n", nova.nome, nova.id);
    estadoAnterior.acao = ACAO_RESERVAR;
}

// Usar peça reservada (desempilhar e jogar)
void usarPecaReservada() {
    if (pilhaVazia(&pilha)) {
        printf("Pilha vazia! Nao ha pecas reservadas para usar.\n");
        return;
    }
    salvarEstado();
    Peca p = desempilhar(&pilha);
    printf("Usou a peca reservada: Tipo %s | ID %d\n", p.nome, p.id);
    estadoAnterior.acao = ACAO_USAR_RESERVADA;
}

// Trocar peça do topo da pilha com a da frente da fila
void trocarTopoPilhaFila() {
    if (pilhaVazia(&pilha)) {
        printf("Pilha vazia! Nao ha peca no topo para trocar.\n");
        return;
    }
    if (fila.tamanho == 0) {
        printf("Fila vazia! Nao ha peca na frente para trocar.\n");
        return;
    }
    salvarEstado();
    Peca pTopo = pilha.pilha[pilha.topo];
    Peca pFrente = fila.fila[fila.frente];
    pilha.pilha[pilha.topo] = pFrente;
    fila.fila[fila.frente] = pTopo;
    printf("Troca realizada entre topo da pilha e frente da fila.\n");
    estadoAnterior.acao = ACAO_TROCAR;
}

// Inverter fila com pilha
void inverterFilaComPilha() {
    if (fila.tamanho == 0 && pilhaVazia(&pilha)) {
        printf("Fila e pilha vazias! Nada para inverter.\n");
        return;
    }
    salvarEstado();

    // Copiar fila para array temporário
    Peca tempFila[TAM_FILA];
    int idx = fila.frente;
    for (int i = 0; i < fila.tamanho; i++) {
        tempFila[i] = fila.fila[idx];
        idx = (idx + 1) % TAM_FILA;
    }

    // Copiar pilha para array temporário
    Peca tempPilha[TAM_PILHA];
    int tamPilha = pilha.topo + 1;
    for (int i = 0; i < tamPilha; i++) {
        tempPilha[i] = pilha.pilha[i];
    }

    // Inverter fila com pilha: fila recebe pilha invertida, pilha recebe fila invertida
    // Inverter pilha para fila
    inicializarFila(&fila);
    for (int i = tamPilha - 1; i >= 0; i--) {
        enfileirar(&fila, tempPilha[i]);
    }

    // Inverter fila para pilha
    inicializarPilha(&pilha);
    for (int i = fila.tamanho - 1; i >= 0; i--) {
        empilhar(&pilha, tempFila[i]);
    }

    printf("Fila e pilha invertidas com sucesso.\n");
    estadoAnterior.acao = ACAO_INVERTER;
}

// Menu principal
void menu() {
    int opcao;
    do {
        mostrarFila(&fila);
        mostrarPilha(&pilha);
        printf("\nMenu:\n");
        printf("1 - Jogar peca\n");
        printf("2 - Reservar peca\n");
        printf("3 - Usar peca reservada\n");
        printf("4 - Trocar topo da pilha com frente da fila\n");
        printf("5 - Desfazer ultima jogada\n");
        printf("6 - Inverter fila com pilha\n");
        printf("0 - Sair\n");
        printf("Escolha uma opcao: ");
        scanf("%d", &opcao);
        getchar(); // limpar buffer

        switch (opcao) {
            case JOGAR_PECA:
                jogarPeca();
                break;
            case RESERVAR_PECA:
                reservarPeca();
                break;
            case USAR_PECA_RESERVADA:
                usarPecaReservada();
                break;
            case TROCAR_TOPO_PILHA_FILA:
                trocarTopoPilhaFila();
                break;
            case DESFAZER_ULTIMA_JOGADA:
                desfazer();
                break;
            case INVERTER_FILA_COM_PILHA:
                inverterFilaComPilha();
                break;
            case SAIR:
                printf("Encerrando o jogo. Ate logo!\n");
                break;
            default:
                printf("Opcao invalida! Tente novamente.\n");
        }
    } while (opcao != SAIR);
}
