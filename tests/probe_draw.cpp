// ---------------------------------------------------------------------------
// Sonda B: existe tabuleiro cheio (42 pecas) sem 4 em linha?
//
// O ramo do empate nos jogos (current_player = 'T') so dispara com o tabuleiro
// cheio, e nenhuma funcao de CBoard o responde. Aqui procura-se, com procura
// aleatoria repetida (semente fixa), uma sequencia de 42 jogadas legais em que
// NENHUMA feche 4 em linha; se aparecer, verifica-se:
//   * com uma referencia independente (refWin/anyWin, escrita para o teste);
//   * com o proprio CBoard::checkWin a cada jogada;
//   * com getRow() == -1 nas 7 colunas no fim.
//
// Nao altera nada do jogo.
// ---------------------------------------------------------------------------
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "cboard.h"

class CPvM { // acesso de leitura via a amizade ja declarada no jogo
public:
    static int turnOf(const CBoard &b) { return b.turn; }
};

static bool refWin(const char g[6][7], int row, int col, char p) {
    static const int dr[4] = {0, 1, 1, 1};
    static const int dc[4] = {1, 0, 1, -1};
    for (int d = 0; d < 4; d++) {
        int n = 1;
        for (int s = 1; s < 4; s++) {
            int r = row + s * dr[d], c = col + s * dc[d];
            if (r >= 0 && r < 6 && c >= 0 && c < 7 && g[r][c] == p) n++; else break;
        }
        for (int s = 1; s < 4; s++) {
            int r = row - s * dr[d], c = col - s * dc[d];
            if (r >= 0 && r < 6 && c >= 0 && c < 7 && g[r][c] == p) n++; else break;
        }
        if (n >= 4) return true;
    }
    return false;
}

// varredura completa: ha 4 em linha em algum sitio do tabuleiro?
static bool anyWin(const char g[6][7]) {
    for (int r = 0; r < 6; r++)
        for (int c = 0; c < 7; c++)
            if (g[r][c] != ' ' && refWin(g, r, c, g[r][c])) return true;
    return false;
}

static void dump(const char g[6][7]) {
    for (int r = 0; r < 6; r++) {
        printf("  |");
        for (int c = 0; c < 7; c++) printf("%c|", g[r][c]);
        printf("\n");
    }
}

int main(int argc, char **argv) {
    int trials = argc > 1 ? atoi(argv[1]) : 200000;
    unsigned long long seed = 987654321ULL;

    for (int t = 0; t < trials; t++) {
        char g[6][7];
        memset(g, ' ', sizeof g);
        int h[7] = {0, 0, 0, 0, 0, 0, 0};
        int seq[42][2];
        bool ok = true;

        for (int turn = 0; turn < 42; turn++) {
            char p = (turn % 2 == 0) ? 'O' : '0';
            int cand[7], nc = 0;
            for (int c = 0; c < 7; c++) {
                if (h[c] >= 6) continue;
                int r = 5 - h[c];
                g[r][c] = p;
                bool w = refWin(g, r, c, p);
                g[r][c] = ' ';
                if (!w) cand[nc++] = c; // jogada que nao fecha 4 em linha
            }
            if (nc == 0) { ok = false; break; } // so restam jogadas vencedoras
            seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
            int c = cand[(seed >> 33) % (unsigned)nc];
            int r = 5 - h[c];
            g[r][c] = p;
            h[c]++;
            seq[turn][0] = c;
            seq[turn][1] = r;
        }
        if (!ok) continue;

        printf("achado: tabuleiro cheio sem vitoria na tentativa %d\n", t);
        if (anyWin(g)) { printf("ERRO: a varredura independente encontrou 4 em linha\n"); return 1; }
        printf("referencia independente: nenhum 4 em linha (42 celulas varridas)\n");
        dump(g);

        CBoard b;
        int wid = 0;
        for (int turn = 0; turn < 42; turn++) {
            char p = (turn % 2 == 0) ? 'O' : '0';
            int c = seq[turn][0];
            b.playPiece(c, p);
            if (b.checkWin(c, 4)) wid++; // o jogo nao veria vitoria nenhuma
        }
        bool full = true;
        for (int c = 0; c < 7; c++) if (b.getRow(c) != -1) full = false;
        printf("replay no CBoard: turn == %d, checkWin verdadeiros == %d (esperado 0), tabuleiro cheio == %s\n",
               CPvM::turnOf(b), wid, full ? "sim" : "nao");
        return (wid == 0 && full && CPvM::turnOf(b) == 42) ? 0 : 1;
    }

    printf("nao encontrei tabuleiro cheio sem vitoria em %d tentativas (inconclusivo)\n", trials);
    return 3;
}
