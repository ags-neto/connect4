# connect4

> Jogo Connect 4 em C++ para consola Windows (6×7, três modos de jogo, três níveis de
> dificuldade): trabalho académico de EDA. Compila como executável de consola Windows e
> **não corre neste sistema**.

## What it is

Trabalho académico de EDA, em C++, entregue como **aplicação de consola Windows**: o
relatório do projecto é `projeto_eda.pdf` e os enunciados são `Projeto.docx` e
`Projeto P.2.docx`. O tabuleiro tem 42 células (6 linhas × 7 colunas), com detecção de
vitória na horizontal, na vertical e nas duas diagonais; três modos (jogador vs máquina,
jogador vs jogador, máquina vs máquina), três níveis de dificuldade da máquina (jogada
aleatória, ganhar/defender de imediato, melhor jogada por pontuação) e leaderboards em
ficheiro de texto, escritos no directório de trabalho.

Código (1691 linhas: 1491 de `.cpp`, 183 de headers e 17 do `.pro`):

| Ficheiro | Linhas | Papel |
|---|---|---|
| `Connect4/main.cpp` | 440 | menus, criação e retoma de jogos, leaderboards (interface) |
| `Connect4/cpvm.cpp` + `.h` | 364 + 52 | jogador vs máquina: jogada, algoritmos, ficheiros |
| `Connect4/cpvp.cpp` + `.h` | 271 + 45 | jogador vs jogador |
| `Connect4/cmvm.cpp` + `.h` | 243 + 47 | máquina vs máquina |
| `Connect4/cboard.cpp` + `.h` | 173 + 39 | tabuleiro: jogar/desfazer peça, vitória, impressão |
| `Connect4/Connect4.pro` | 17 | projecto qmake (`TEMPLATE = app`, `CONFIG += console c++11`, `CONFIG -= qt`) |

A lógica do tabuleiro está separada da interface: `CBoard` (colocar peça, desfazer,
verificar vitória) não faz I/O de menus; a interface (menus, cores, `cls`/`pause`) está
em `main.cpp` e nas classes de jogo, que usam `CBoard`.

### O que depende de Windows

- `Connect4/cboard.h:3` — `#include <windows.h>` (único include da API do sistema).
- `SetConsoleTextAttribute` (13 ocorrências: `cboard.cpp` 4, `cmvm.cpp` 3, `cpvm.cpp` 3,
  `cpvp.cpp` 3) com `HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE)` — cor das peças e
  das mensagens.
- `system("cls")` (31 ocorrências: `main.cpp` 28, `cmvm.cpp` 1, `cpvm.cpp` 1, `cpvp.cpp` 1)
  e `system("pause")` (10: `main.cpp` 6, `cmvm.cpp` 2, `cpvm.cpp` 1, `cpvp.cpp` 1).
- **Não** há `conio.h`, `Sleep`, `kbhit` nem códigos de cor ANSI.

O executável importa `KERNEL32.dll` e `msvcrt.dll`: é uma aplicação de consola Windows
(`main`/`system`/`cls`/`pause`) e não corre em Linux.

### Estado do código (o que está morto ou frágil)

- `CBoard::printBoardAdvanced()` (`cboard.cpp:155`) está marcada `// TODO` e **nunca é
  chamada**: código morto (declarada em `cboard.h:36`).
- `CBoard::playPiece` (`cboard.cpp:45`) e `CBoard::checkWin` (`cboard.cpp:63`) **não
  validam a coluna**: em coluna cheia `playPiece` escreve fora do tabuleiro e `checkWin`
  lê fora. Todos os chamadores do jogo validam com `getRow(col) != -1` antes, portanto o
  caminho não é atingível pelos menus — mas a validação vive na interface, não na lógica.
  Prova em `## Tests`.
- `CBoard::checkWin(col, num)` responde "a linha/coluna/diagonal dada contém `num` peças
  desta cor", não "esta jogada fechou `num`". No jogo é equivalente, porque a partida
  acaba na primeira vitória.
- `Connect4/Connect4.pro.user` e `Connect4/Connect4.pro.user.8c2f6f0` (configuração do Qt
  Creator) e o directório `build-Connect4-Desktop_Qt_5_15_2_MinGW_64_bit-Debug/` estão
  versionados: esse directório tem um Makefile gerado e saída de execução
  (`leaderboard_pvp.txt`, com nomes de jogadores). O `.gitignore` passou a cobri-los, mas
  os que já estavam no índice continuam versionados (não foram removidos).
- Não há blocos grandes de código comentado.

## Requirements

- Compilador cruzado MinGW-w64 (o alvo é Windows):
  `sudo apt-get install -y g++-mingw-w64-x86-64` — testado com GCC 14.2 (`x86_64-w64-mingw32-g++`).
- `qmake` (opcional, para usar o `.pro`): `sudo apt-get install -y qt5-qmake`. O `.pro`
  declara `CONFIG -= qt` e não liga nenhum módulo Qt — o Qt foi só a ferramenta de
  construção original, e o executável **não** importa DLLs do Qt.
- Sem bibliotecas de terceiros no repositório.
- Para correr: Windows (a aplicação usa `cls`/`pause` e a API de consola do Windows).

## Install / Build

O projecto vem do Qt Creator. O kit do build versionado **não** é inferência: o
`build-Connect4-Desktop_Qt_5_15_2_MinGW_64_bit-Debug/Makefile` (gerado por qmake) diz
`D:\QtCreator\5.15.2\mingw81_64\bin\qmake.exe -o Makefile ..\Connect4\Connect4.pro -spec
win32-g++ "CONFIG+=debug"`, e `Connect4/Connect4.pro.user` diz
`qt.qt5.5152.win64_mingw81_kit`. Ou seja: **Qt 5.15.2 + MinGW GCC 8.1 64-bit, build Debug**;
o `mingw81_64` no caminho do qmake é a versão do MinGW, que o nome do directório de build
(`..._MinGW_64_bit-Debug`) não dizia.

Compilação cruzada neste sistema (Debian 13 arm64), verificada:

```sh
x86_64-w64-mingw32-g++ -std=c++11 -Wall -O2 Connect4/*.cpp -o connect4.exe
# 0 erros, 4 avisos -Wall; connect4.exe = PE32+ executable for MS Windows (console), x86-64

# executável autónomo, sem precisar de libstdc++-6.dll / libgcc_s_seh-1.dll ao lado:
x86_64-w64-mingw32-g++ -std=c++11 -Wall -O2 -static Connect4/*.cpp -o connect4.exe
```

Com `qmake` (o caminho que o `.pro` espera; o Qt 5.15.2 não é preciso para compilar, basta
qmake + MinGW no `PATH`):

```sh
mkdir build && cd build
/usr/lib/qt5/bin/qmake ../Connect4/Connect4.pro -spec win32-g++
make            # -> release/Connect4.exe (PE32+ console x86-64, stripped)
```

No Windows: abrir `Connect4/Connect4.pro` no Qt Creator com o kit
*Desktop Qt 5.15.2 MinGW 64-bit* e correr.

Os avisos do `-Wall` (4, iguais nos dois caminhos) são variáveis possivelmente não
inicializadas: `cpvm.cpp:79` (`play`), `main.cpp:188`, `main.cpp:257` e `main.cpp:349`
(`starter_int`).

## Usage

Aplicação de consola Windows, sem argumentos. Menu principal:
`1) New Game`, `2) Resume Game`, `3) Leaderboards`, `4) Commands`, `0) Quit`.
Em jogo, a jogada é o número da coluna (1–7). Antes de cada modo pede nomes e o
jogador que começa; nos modos com máquina pede a dificuldade (1–3).

Ficheiros escritos no directório de trabalho: `savedgame_pvm.txt`, `savedgame_pvp.txt`,
`savedgame_mvm.txt` (jogo a retomar) e `leaderboard_pvm.txt`, `leaderboard_pvp.txt`
(top 10, ordenado por vitórias − derrotas). A opção `2) Resume Game` depende desses
ficheiros.

**Não foi executado aqui**: não corre em Linux (é um executável de consola Windows, com
`cls`/`pause` e cor via `SetConsoleTextAttribute`). Não há `wine` neste sistema.

## Tests

O código do jogo **não foi alterado**. `tests/run-tests.sh` faz três coisas: (1) compila
`Connect4/cboard.cpp` para Linux usando `tests/shim/windows.h` (um `windows.h` mínimo com
`GetStdHandle`/`SetConsoleTextAttribute` que só o build de teste usa) e corre
`tests/test_cboard.cpp`; (2) corre duas sondas com AddressSanitizer e uma procura de
tabuleiro cheio sem vitória; (3) faz o build real para Windows com mingw-w64. O binário
do jogo é escrito apenas em `tests/build/` (ignorado pelo git).

```sh
$ ./tests/run-tests.sh
================== 1/3  logica do tabuleiro (CBoard) para Linux + bateria de testes
  $ g++ -std=c++11 -Wall -Wextra -I Connect4 -I tests/shim Connect4/cboard.cpp tests/test_cboard.cpp -o tests/build/test_cboard
  (compilado sem avisos)
connect4 - testes da logica do tabuleiro (CBoard)

[dimensoes e convencao de indice]
  ok   MAX_C == 7 colunas
  ok   MAX_R == 6 linhas
  ok   SIZE == 43 == 6*7+1 (turn e 1-based)

[getRow / playPiece: gravidade e empilhamento]
  ok   coluna vazia: getRow() == 5 (fundo)
  ok   playPiece incrementa turn
  ok   a peca cai no fundo (linha 5)
  ok   a posicao anterior fica intacta (historico)
  ok   getRow desce uma linha depois da jogada
  ok   a segunda peca empilha em cima
  ok   a peca de baixo mantem-se

[coluna cheia: 6 pecas, getRow() == -1]
  [... "getRow desce ao encher a coluna" x6 ...]
  ok   coluna cheia: getRow() == -1 (jogada ilegal detetavel)

[vitoria horizontal]
  ok   1 peca: sem vitoria
  ok   3 em linha horizontal: sem vitoria com num=4
  ok   3 em linha horizontal: vitoria com num=3 (parametro num)
  ok   4 em linha horizontal: VITORIA

[vitoria vertical]
  ok   3 na vertical: sem vitoria
  ok   4 na vertical: VITORIA
  ok   2 empilhadas: vitoria com num=2
  ok   2 empilhadas: sem vitoria com num=3

[vitoria diagonal NE (sobe para a direita), ramo esquerdo do codigo]
  ok   3 na diagonal NE: sem vitoria
  ok   4 na diagonal NE: VITORIA

[vitoria diagonal NE, ramo direito do codigo]
  ok   diagonal NE incompleta: sem vitoria
  ok   4 na diagonal NE (ramo direito): VITORIA

[vitoria diagonal SE (desce para a direita), ramo esquerdo do codigo]
  ok   3 na diagonal SE: sem vitoria
  ok   4 na diagonal SE: VITORIA

[vitoria diagonal SE, ramo direito do codigo]
  ok   diagonal SE incompleta: sem vitoria
  ok   4 na diagonal SE (ramo direito): VITORIA

[sem falsos positivos]
  ok   4 pecas na horizontal mas com falha na coluna 3: sem vitoria
  ok   coluna alternada (0,O,0,O): sem vitoria
  ok   coluna alternada: as pecas iguais nao ficam contiguas (sem num=2)
  ok   coluna alternada: num=1 devolve true

[historico, undoPiece e advancePiece (usados na defesa/retomar jogo)]
  ok   turn == 3 apos tres jogadas
  ok   undoPiece(2) recua duas jogadas
  ok   a posicao de turn==1 mantem-se
  ok   advancePiece(2) volta ao turn 3
  ok   o estado avancado e o mesmo de antes do undo

[simulateWin: testa sem jogar (usado pela maquina)]
  ok   simulateWin numa jogada vencedora devolve true
  ok   simulateWin restaura o turn
  ok   simulateWin nao deixa a peca no tabuleiro
  ok   a celula simulada volta a ficar vazia
  ok   simulateWin numa coluna sem vitoria devolve false
  ok   simulateWin (false) tambem restaura o turn
  ok   simulateWin com a cor do adversario na mesma coluna: false (a defesa e outro caminho)
  ok   e restaura o turn outra vez

[capacidade: 42 pecas, todas as colunas cheias]
  ok   42 jogadas cabem no tabuleiro
  ok   turn == 42 depois de encher
  ok   as 7 colunas ficam cheias (getRow() == -1)
  ok   a condicao de empate dos jogos (turn+1 == SIZE) fica atingida aos 42 - mas o ramo do empate vive na interface, nao em CBoard

[semantica de checkWin: 'a linha contem 4', nao 'esta jogada fechou 4']
  ok   4 em linha em (5,3..6): vitoria (esperado)
  nota: checkWin(0,4) devolve 1 para a peca em (5,0), que sozinha nao fecha 4.
  ok   devolve true porque a linha 5 ja tinha 4 'O' noutro sitio (correto no jogo: a partida teria acabado na jogada anterior)

[prova diferencial: 4000 partidas aleatorias, CBoard vs referencia independente]
  partidas=4000 | jogadas comparadas=84909 | vitorias=3988 | sem vitoria (tabuleiro cheio)=12
  max pecas numa partida=42 | pecas nos empates=42
  ok   checkWin concorda com a referencia independente em todas as jogadas
  ok   o tabuleiro interno do CBoard espelha sempre a posicao esperada
  ok   nenhuma partida passa das 42 pecas

59 verificacoes, 0 falhas
```

(excertos do log real: `[...]` marca cortes, e no passo 3 os blocos de `warning:` estão
resumidos; o log completo tem 264 linhas e é reproduzido pelo script)

Cobertura: dimensões do tabuleiro, gravidade e empilhamento, coluna cheia (`getRow == -1`),
vitória horizontal, vertical e nas **quatro** diagonais (os dois ramos de cada ciclo de
`checkWin`), ausência de falsos positivos, histórico (`undoPiece`/`advancePiece`),
`simulateWin` (a máquina testa jogadas sem as deixar no tabuleiro), capacidade de 42 peças
**e empate com tabuleiro cheio** — a prova diferencial encontrou 12 partidas de 4000 que
terminam com o tabuleiro cheio e nenhuma vitória, com o `checkWin` do jogo a concordar com
uma referência independente em todas as 84909 jogadas.

O que **não** ficou testado: tudo o que está dentro da interface — menus, cores
(`SetConsoleTextAttribute`), `cls`/`pause`, leitura de `cin`, gravação/leitura dos
`*.txt`, leaderboards (`saveScore`, `selectionSort`) e as heurísticas
(`randomPlay`/`winNextPlay`/`defendNextPlay`/`bestPlay`) — porque vive em ficheiros que
compilam mas não correm sem a consola Windows e sem interacção. Os testes ligam apenas
`cboard.cpp`; **nenhuma** linha de `main.cpp`, `cpvm.cpp`, `cpvp.cpp` ou `cmvm.cpp` foi
executada. O ramo do empate (`current_player = 'T'`) também não foi executado: só se
provou que o estado que o dispara (42 peças, sem vitória) existe e é atingível.

As duas limitações não validadas de `CBoard` ficaram provadas com AddressSanitizer
(`tests/probe_oob.cpp`; não são atingíveis pelos menus do jogo, porque todos os
chamadores verificam `getRow(...) != -1`):

```sh
$ tests/build/probe_oob checkwin-coluna-vazia
sonda A1: checkWin(0, 4) numa coluna vazia (coluna 0 sem pecas)
  getRow(0) == 5  ->  a funcao vai ler a linha 6 (MAX_R == 6)
ERROR: AddressSanitizer: heap-buffer-overflow
READ of size 8
    #0 CBoard::checkWin(int, int) const Connect4/cboard.cpp:65
SUMMARY: AddressSanitizer: heap-buffer-overflow Connect4/cboard.cpp:65 in CBoard::checkWin(int, int) const

$ tests/build/probe_oob playpiece-coluna-cheia
sonda A2: playPiece(0, 'O') numa coluna cheia
  getRow(0) == -1  ->  escreve boards[turn+1][-1][0]
ERROR: AddressSanitizer: heap-buffer-overflow
READ of size 8
    #0 CBoard::playPiece(int, char) Connect4/cboard.cpp:49
SUMMARY: AddressSanitizer: heap-buffer-overflow Connect4/cboard.cpp:49 in CBoard::playPiece(int, char)
```

E a sonda do empate (`tests/probe_draw.cpp`, com referência independente e replay no
`CBoard`) encontrou, na 1.ª tentativa, um tabuleiro cheio sem nenhum 4 em linha:

```
|O|0|0|O|O|0|0|
|0|O|0|O|0|0|O|
|O|0|O|0|O|0|O|
|O|0|O|0|O|O|0|
|0|O|0|O|O|0|0|
|O|0|O|0|0|O|O|
replay no CBoard: turn == 42, checkWin verdadeiros == 0 (esperado 0), tabuleiro cheio == sim
```

Build real para Windows (passo 3 de `run-tests.sh`):

```sh
$ x86_64-w64-mingw32-g++ -std=c++11 -Wall -O2 Connect4/*.cpp -o tests/build/connect4.exe
  codigo de saida: 0
  avisos -Wall: 4   (cpvm.cpp:79 'play'; main.cpp:188, 257, 349 'starter_int')
tests/build/connect4.exe: PE32+ executable for MS Windows 5.02 (console), x86-64, 18 sections
  testes da logica: PASS
```

### Proposta (não aplicada)

Para a lógica ser testável sem a consola Windows, o mínimo era: tirar `#include
<windows.h>` e as cores de `cboard.cpp` (deixar a impressão sem `SetConsoleTextAttribute`
ou passá-la a um parâmetro) e trocar `system("cls")`/`system("pause")` por funções da
interface. Não foi feito — o código do jogo não foi alterado. Os menus e a leitura de
`cin` continuariam a exigir interacção; para testar os modos de jogo e os
leaderboards era preciso injectar entrada/saída.

## Structure

```
Connect4/                          codigo do jogo (nao alterado)
  Connect4.pro                     projecto qmake (CONFIG -= qt)
  main.cpp                         menus, criacao/retoma de jogos, leaderboards
  cboard.{h,cpp}                   tabuleiro: getRow, playPiece, undo/advance, checkWin
  cpvm.{h,cpp}                     jogador vs maquina
  cpvp.{h,cpp}                     jogador vs jogador
  cmvm.{h,cpp}                     maquina vs maquina
  Connect4.pro.user*               configuracao do Qt Creator (versionada)
build-Connect4-Desktop_Qt_5_15_2_MinGW_64_bit-Debug/
                                   Makefile gerado e saida de execucao (versionados)
tests/                             verificacao (nao entra no build do jogo)
  run-tests.sh                     runner (logica + sondas ASan + build Windows)
  test_cboard.cpp                  59 verificacoes da logica do tabuleiro
  shim/windows.h                   shim minimo da API de consola, so para o teste
  probe_oob.cpp                    limites nao validados de CBoard, com ASan
  probe_draw.cpp                   procura de tabuleiro cheio sem vitoria
  build/                           binarios do teste (ignorado pelo git)
Projeto.docx, Projeto P.2.docx     enunciados do trabalho
projeto_eda.pdf                    relatorio do trabalho
LICENSE
README.md                          este ficheiro
```

## License

MIT — ver o ficheiro [`LICENSE`](LICENSE). O trabalho é de co-autoria:
André Guilherme dos Santos Neto e Inês Jorge da Silva e Ferreira.

