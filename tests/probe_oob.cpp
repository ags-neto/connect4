// ---------------------------------------------------------------------------
// Sonda A: prova (com AddressSanitizer) de que CBoard NAO valida duas coisas
// que a interface tem de validar por si:
//
//   1. checkWin(col, n) numa coluna VAZIA -> getRow(col) == 5, logo
//      row == 6 e ele le boards[turn][6][col], fora de MAX_R == 6.
//   2. playPiece(col, p) numa coluna CHEIA -> getRow(col) == -1, logo escreve
//      boards[++turn][-1][col].
//
// Nenhuma das duas e alcancavel a partir dos menus do jogo (todos os caminhos
// de leitura/escrita passam por getRow(...) != -1), mas ficam aqui como prova
// objetiva de que a validacao vive na interface, nao na logica.
//
// Sonda de teste: nao altera nada do jogo.
// ---------------------------------------------------------------------------
#include <cstdio>
#include <cstring>
#include "cboard.h"

int main(int argc, char **argv) {
    const char *mode = argc > 1 ? argv[1] : "";
    CBoard b;

    if (!strcmp(mode, "checkwin-coluna-vazia")) {
        printf("sonda A1: checkWin(0, 4) numa coluna vazia (coluna 0 sem pecas)\n");
        printf("  getRow(0) == %d  ->  a funcao vai ler a linha %d (MAX_R == 6)\n",
               b.getRow(0), b.getRow(0) + 1);
        fflush(stdout);
        bool w = b.checkWin(0, 4);
        printf("  devolveu %d (nao devia ser alcancavel)\n", (int)w);
        return 0;
    }

    if (!strcmp(mode, "playpiece-coluna-cheia")) {
        for (int i = 0; i < CBoard::MAX_R; i++) b.playPiece(0, 'O');
        printf("sonda A2: playPiece(0, 'O') numa coluna cheia\n");
        printf("  getRow(0) == %d  ->  escreve boards[turn+1][-1][0]\n", b.getRow(0));
        fflush(stdout);
        b.playPiece(0, 'O');
        printf("  nao rebentou (o dano seria silencioso)\n");
        return 0;
    }

    fprintf(stderr, "uso: %s checkwin-coluna-vazia | playpiece-coluna-cheia\n", argv[0]);
    return 2;
}
