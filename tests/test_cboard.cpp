// ---------------------------------------------------------------------------
// Bateria de testes da logica do Connect4 (classe CBoard, Connect4/cboard.cpp).
//
// Regras desta bateria:
//   * NAO altera o codigo do jogo: nada em Connect4/ e tocado;
//   * o unico intermediario e tests/shim/windows.h, que substitui <windows.h>
//     para que cboard.cpp (que usa GetStdHandle/SetConsoleTextAttribute) possa
//     ser compilado no Linux; o shim so existe no build de teste;
//   * CBoard declara "friend class CPvM", por isso define-se aqui um CPvM
//     minimo para LER o estado privado (turn, boards). O binario de teste liga
//     apenas cboard.o + este ficheiro, pelo que nao ha conflito com o CPvM
//     verdadeiro (que nem sequer e compilado aqui).
// ---------------------------------------------------------------------------
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "cboard.h"

// Acesso de leitura ao estado privado, pela amizade que o jogo ja declara.
class CPvM {
public:
    static int turnOf(const CBoard &b) { return b.turn; }
    static char cellOf(const CBoard &b, int s, int r, int c) { return b.boards[s][r][c]; }
};

static int g_checks = 0;
static int g_fail = 0;

static void check(bool cond, const char *what) {
    g_checks++;
    if (cond) {
        printf("  ok   %s\n", what);
    } else {
        g_fail++;
        printf("  FAIL %s\n", what);
    }
}

static void section(const char *t) { printf("\n[%s]\n", t); }

// ---------------------------------------------------------------------------
// Referencia independente (escrita para o teste, nao vem do jogo):
// a peca em (row,col) participa em 4 em linha da cor p?
// ---------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------
static void t_dimensoes() {
    section("dimensoes e convencao de indice");
    check(CBoard::MAX_C == 7, "MAX_C == 7 colunas");
    check(CBoard::MAX_R == 6, "MAX_R == 6 linhas");
    check(CBoard::SIZE == 43, "SIZE == 43 == 6*7+1 (turn e 1-based)");
}

static void t_posicao_da_peca() {
    section("getRow / playPiece: gravidade e empilhamento");
    CBoard b;
    check(b.getRow(0) == 5 && b.getRow(6) == 5, "coluna vazia: getRow() == 5 (fundo)");
    b.playPiece(0, 'O');
    check(CPvM::turnOf(b) == 1, "playPiece incrementa turn");
    check(CPvM::cellOf(b, 1, 5, 0) == 'O', "a peca cai no fundo (linha 5)");
    check(CPvM::cellOf(b, 0, 5, 0) == ' ', "a posicao anterior fica intacta (historico)");
    check(b.getRow(0) == 4, "getRow desce uma linha depois da jogada");
    b.playPiece(0, '0');
    check(CPvM::cellOf(b, 2, 4, 0) == '0', "a segunda peca empilha em cima");
    check(CPvM::cellOf(b, 2, 5, 0) == 'O', "a peca de baixo mantem-se");

    section("coluna cheia: 6 pecas, getRow() == -1");
    CBoard f;
    for (int i = 0; i < 6; i++) {
        f.playPiece(3, (i % 2) ? '0' : 'O');
        check(f.getRow(3) == 5 - (i + 1), "getRow desce ao encher a coluna");
    }
    check(f.getRow(3) == -1, "coluna cheia: getRow() == -1 (jogada ilegal detetavel)");
}

static void t_horizontal() {
    section("vitoria horizontal");
    CBoard b;
    b.playPiece(0, 'O');
    check(!b.checkWin(0, 4), "1 peca: sem vitoria");
    b.playPiece(1, 'O');
    b.playPiece(2, 'O');
    check(!b.checkWin(2, 4), "3 em linha horizontal: sem vitoria com num=4");
    check(b.checkWin(2, 3), "3 em linha horizontal: vitoria com num=3 (parametro num)");
    b.playPiece(3, 'O');
    check(b.checkWin(3, 4), "4 em linha horizontal: VITORIA");
}

static void t_vertical() {
    section("vitoria vertical");
    CBoard b;
    for (int i = 0; i < 3; i++) b.playPiece(5, 'O');
    check(!b.checkWin(5, 4), "3 na vertical: sem vitoria");
    b.playPiece(5, 'O');
    check(b.checkWin(5, 4), "4 na vertical: VITORIA");
    CBoard w;
    w.playPiece(2, 'O'); w.playPiece(2, 'O');
    check(w.checkWin(2, 2), "2 empilhadas: vitoria com num=2");
    check(!w.checkWin(2, 3), "2 empilhadas: sem vitoria com num=3");
}

static void t_diagonal_ne() {
    section("vitoria diagonal NE (sobe para a direita), ramo esquerdo do codigo");
    CBoard b;
    b.playPiece(0, 'O');                                    // (5,0)
    b.playPiece(1, '0'); b.playPiece(1, 'O');               // (5,1) (4,1)
    b.playPiece(2, '0'); b.playPiece(2, '0'); b.playPiece(2, 'O'); // (3,2)
    check(!b.checkWin(2, 4), "3 na diagonal NE: sem vitoria");
    b.playPiece(3, '0'); b.playPiece(3, '0'); b.playPiece(3, '0'); b.playPiece(3, 'O'); // (2,3)
    check(b.checkWin(3, 4), "4 na diagonal NE: VITORIA");
}

static void t_diagonal_ne_direita() {
    section("vitoria diagonal NE, ramo direito do codigo");
    CBoard b;
    b.playPiece(3, 'O');                                    // (5,3)
    b.playPiece(4, '0'); b.playPiece(4, 'O');               // (5,4) (4,4)
    b.playPiece(5, '0'); b.playPiece(5, '0'); b.playPiece(5, 'O'); // (3,5)
    check(!b.checkWin(5, 4), "diagonal NE incompleta: sem vitoria");
    b.playPiece(6, '0'); b.playPiece(6, '0'); b.playPiece(6, '0'); b.playPiece(6, 'O'); // (2,6)
    check(b.checkWin(6, 4), "4 na diagonal NE (ramo direito): VITORIA");
}

static void t_diagonal_se() {
    section("vitoria diagonal SE (desce para a direita), ramo esquerdo do codigo");
    CBoard b;
    b.playPiece(3, 'O');                                    // (5,3)
    b.playPiece(2, '0'); b.playPiece(2, 'O');               // (5,2) (4,2)
    b.playPiece(1, '0'); b.playPiece(1, '0'); b.playPiece(1, 'O'); // (3,1)
    check(!b.checkWin(1, 4), "3 na diagonal SE: sem vitoria");
    b.playPiece(0, '0'); b.playPiece(0, '0'); b.playPiece(0, '0'); b.playPiece(0, 'O'); // (2,0)
    check(b.checkWin(0, 4), "4 na diagonal SE: VITORIA");
}

static void t_diagonal_se_direita() {
    section("vitoria diagonal SE, ramo direito do codigo");
    CBoard b;
    for (int i = 0; i < 5; i++) b.playPiece(1, '0');
    b.playPiece(1, 'O');                                    // (0,1)
    for (int i = 0; i < 4; i++) b.playPiece(2, '0');
    b.playPiece(2, 'O');                                    // (1,2)
    for (int i = 0; i < 3; i++) b.playPiece(3, '0');
    b.playPiece(3, 'O');                                    // (2,3)
    for (int i = 0; i < 2; i++) b.playPiece(4, '0');
    check(!b.checkWin(3, 4), "diagonal SE incompleta: sem vitoria");
    b.playPiece(4, 'O');                                    // (3,4)
    check(b.checkWin(4, 4), "4 na diagonal SE (ramo direito): VITORIA");
}

static void t_sem_falso_positivo() {
    section("sem falsos positivos");
    CBoard b;
    b.playPiece(0, 'O'); b.playPiece(1, 'O'); b.playPiece(2, 'O'); b.playPiece(4, 'O');
    check(!b.checkWin(4, 4), "4 pecas na horizontal mas com falha na coluna 3: sem vitoria");
    CBoard v;
    v.playPiece(6, '0'); v.playPiece(6, 'O'); v.playPiece(6, '0'); v.playPiece(6, 'O');
    check(!v.checkWin(6, 4), "coluna alternada (0,O,0,O): sem vitoria");
    check(!v.checkWin(6, 2), "coluna alternada: as pecas iguais nao ficam contiguas (sem num=2)");
    check(v.checkWin(6, 1), "coluna alternada: num=1 devolve true");
}

static void t_historico_undo_advance() {
    section("historico, undoPiece e advancePiece (usados na defesa/retomar jogo)");
    CBoard b;
    b.playPiece(0, 'O');
    b.playPiece(1, '0');
    b.playPiece(1, 'O');
    check(CPvM::turnOf(b) == 3, "turn == 3 apos tres jogadas");
    b.undoPiece(2);
    check(CPvM::turnOf(b) == 1, "undoPiece(2) recua duas jogadas");
    check(CPvM::cellOf(b, 1, 5, 0) == 'O', "a posicao de turn==1 mantem-se");
    b.advancePiece(2);
    check(CPvM::turnOf(b) == 3, "advancePiece(2) volta ao turn 3");
    check(CPvM::cellOf(b, 3, 4, 1) == 'O', "o estado avancado e o mesmo de antes do undo");
}

static void t_simulate_win() {
    section("simulateWin: testa sem jogar (usado pela maquina)");
    CBoard b;
    b.playPiece(0, 'O'); b.playPiece(1, 'O'); b.playPiece(2, 'O');
    int t0 = CPvM::turnOf(b);
    check(b.simulateWin(3, 'O'), "simulateWin numa jogada vencedora devolve true");
    check(CPvM::turnOf(b) == t0, "simulateWin restaura o turn");
    check(b.getRow(3) == 5, "simulateWin nao deixa a peca no tabuleiro");
    check(CPvM::cellOf(b, t0, 5, 3) == ' ', "a celula simulada volta a ficar vazia");
    check(!b.simulateWin(6, 'O'), "simulateWin numa coluna sem vitoria devolve false");
    check(CPvM::turnOf(b) == t0, "simulateWin (false) tambem restaura o turn");
    check(!b.simulateWin(3, '0'),
          "simulateWin com a cor do adversario na mesma coluna: false (a defesa e outro caminho)");
    check(CPvM::turnOf(b) == t0, "e restaura o turn outra vez");
}

static void t_limite_42_pecas() {
    section("capacidade: 42 pecas, todas as colunas cheias");
    CBoard b;
    int n = 0;
    for (int col = 0; col < 7; col++)
        for (int k = 0; k < 6; k++) { b.playPiece(col, (n % 2) ? '0' : 'O'); n++; }
    check(n == 42, "42 jogadas cabem no tabuleiro");
    check(CPvM::turnOf(b) == 42, "turn == 42 depois de encher");
    bool allfull = true;
    for (int c = 0; c < 7; c++) if (b.getRow(c) != -1) allfull = false;
    check(allfull, "as 7 colunas ficam cheias (getRow() == -1)");
    check(CPvM::turnOf(b) + 1 == CBoard::SIZE,
          "a condicao de empate dos jogos (turn+1 == SIZE) fica atingida aos 42 - "
          "mas o ramo do empate vive na interface, nao em CBoard");
}

static void t_quirk_checkwin() {
    section("semantica de checkWin: 'a linha contem 4', nao 'esta jogada fechou 4'");
    CBoard b;
    b.playPiece(3, 'O'); b.playPiece(4, 'O'); b.playPiece(5, 'O'); b.playPiece(6, 'O');
    check(b.checkWin(6, 4), "4 em linha em (5,3..6): vitoria (esperado)");
    b.playPiece(0, 'O'); // agora a linha 5 tem 5 'O', mas a peca nova nao fecha nada de novo
    printf("  nota: checkWin(0,4) devolve %d para a peca em (5,0), que sozinha nao fecha 4.\n",
           (int)b.checkWin(0, 4));
    check(b.checkWin(0, 4),
          "devolve true porque a linha 5 ja tinha 4 'O' noutro sitio (correto no jogo: "
          "a partida teria acabado na jogada anterior)");
}

static void t_diferencial() {
    section("prova diferencial: 4000 partidas aleatorias, CBoard vs referencia independente");
    unsigned long long seed = 20261008ULL;
    long long cmp = 0, state = 0, diverg = 0;
    int games = 4000, wins = 0, ties = 0, maxpieces = 0, tie_pieces = 0;
    char g[6][7];
    for (int game = 0; game < games; game++) {
        CBoard b;
        memset(g, ' ', sizeof g);
        int h[7] = {0, 0, 0, 0, 0, 0, 0};
        int turn = 0;
        bool won = false;
        while (turn + 1 < CBoard::SIZE) {
            seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
            int col = (int)((seed >> 33) % 7);
            if (h[col] >= 6) continue; // coluna cheia: jogada ilegal, como no jogo
            char p = (turn % 2 == 0) ? 'O' : '0';
            int row = 5 - h[col];
            b.playPiece(col, p);
            g[row][col] = p;
            h[col]++;
            turn++;
            // estado interno tem de espelhar o tabuleiro do teste
            if (CPvM::cellOf(b, turn, row, col) != p || CPvM::cellOf(b, turn - 1, row, col) != ' ') state++;
            bool got = b.checkWin(col, 4);
            bool exp = refWin(g, row, col, p);
            if (got != exp) {
                diverg++;
                if (diverg <= 5)
                    printf("  divergencia: jogo=%d jogada=%d col=%d CBoard=%d ref=%d\n",
                           game, turn, col, (int)got, (int)exp);
            }
            cmp++;
            if (got) { won = true; break; }
        }
        if (won) wins++;
        else { ties++; tie_pieces = turn; }
        if (turn > maxpieces) maxpieces = turn;
    }
    printf("  partidas=%d | jogadas comparadas=%lld | vitorias=%d | sem vitoria (tabuleiro cheio)=%d\n",
           games, cmp, wins, ties);
    printf("  max pecas numa partida=%d | pecas nos empates=%d\n", maxpieces, tie_pieces);
    check(diverg == 0, "checkWin concorda com a referencia independente em todas as jogadas");
    check(state == 0, "o tabuleiro interno do CBoard espelha sempre a posicao esperada");
    check(maxpieces <= 42, "nenhuma partida passa das 42 pecas");
}

int main() {
    printf("connect4 - testes da logica do tabuleiro (CBoard)\n");
    t_dimensoes();
    t_posicao_da_peca();
    t_horizontal();
    t_vertical();
    t_diagonal_ne();
    t_diagonal_ne_direita();
    t_diagonal_se();
    t_diagonal_se_direita();
    t_sem_falso_positivo();
    t_historico_undo_advance();
    t_simulate_win();
    t_limite_42_pecas();
    t_quirk_checkwin();
    t_diferencial();
    printf("\n%d verificacoes, %d falhas\n", g_checks, g_fail);
    return g_fail == 0 ? 0 : 1;
}
