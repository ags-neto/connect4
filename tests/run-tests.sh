#!/usr/bin/env bash
# ---------------------------------------------------------------------------
# run-tests.sh — verificacao do connect4. NAO altera o codigo do jogo.
#
#   1. compila a logica do tabuleiro (Connect4/cboard.cpp) para Linux, usando
#      tests/shim/windows.h, e corre a bateria tests/test_cboard.cpp;
#   2. corre as sondas com AddressSanitizer (tests/probe_oob.cpp) e a procura de
#      tabuleiro cheio sem vitoria (tests/probe_draw.cpp);
#   3. faz o build real para Windows com mingw-w64 (o jogo e uma aplicacao de
#      consola Windows) e mostra os avisos.
#
# O binario produzido em 3 nao corre aqui: e um .exe de Windows.
# Saida: 0 se os testes da logica passarem.
# ---------------------------------------------------------------------------
set -u
here="$(cd "$(dirname "$0")" && pwd)"
root="$(cd "$here/.." && pwd)"
cd "$root" || exit 1
out="$here/build"
mkdir -p "$out"

rc_all=0
say() { printf '\n================== %s\n' "$1"; }
plain() { # corre um comando sem contar o codigo de saida como falha
    echo "  \$ $*"
    "$@"
    echo "  (codigo de saida: $?)"
}

say "1/3  logica do tabuleiro (CBoard) para Linux + bateria de testes"
echo "  \$ g++ -std=c++11 -Wall -Wextra -I Connect4 -I tests/shim Connect4/cboard.cpp tests/test_cboard.cpp -o tests/build/test_cboard"
g++ -std=c++11 -Wall -Wextra -I Connect4 -I "$here/shim" \
    Connect4/cboard.cpp "$here/test_cboard.cpp" -o "$out/test_cboard"
if [ $? -ne 0 ]; then
    echo "  compilacao dos testes FALHOU"
    rc_all=1
else
    echo "  (compilado sem avisos)"
    "$out/test_cboard"
    if [ $? -ne 0 ]; then rc_all=1; fi
fi

say "2/3  sondas: limites nao validados em CBoard (AddressSanitizer)"
g++ -std=c++11 -g -fsanitize=address -I Connect4 -I "$here/shim" \
    Connect4/cboard.cpp "$here/probe_oob.cpp" -o "$out/probe_oob"
if [ $? -ne 0 ]; then
    echo "  compilacao da sonda falhou"
else
    echo "  --- A1: checkWin() numa coluna VAZIA"
    plain "$out/probe_oob" checkwin-coluna-vazia
    echo "  --- A2: playPiece() numa coluna CHEIA"
    plain "$out/probe_oob" playpiece-coluna-cheia
fi

say "2b/3  sonda: existe tabuleiro cheio (42 pecas) sem vitoria?"
g++ -std=c++11 -O2 -I Connect4 -I "$here/shim" \
    Connect4/cboard.cpp "$here/probe_draw.cpp" -o "$out/probe_draw"
if [ $? -ne 0 ]; then
    echo "  compilacao da sonda falhou"
else
    echo "  \$ tests/build/probe_draw 50000"
    "$out/probe_draw" 50000
    echo "  (codigo de saida: $?)"
fi

say "3/3  build real para Windows (mingw-w64) — aplicacao de consola Windows"
if command -v x86_64-w64-mingw32-g++ >/dev/null 2>&1; then
    log="$out/build-windows.log"
    echo "  \$ x86_64-w64-mingw32-g++ -std=c++11 -Wall -O2 Connect4/*.cpp -o tests/build/connect4.exe"
    x86_64-w64-mingw32-g++ -std=c++11 -Wall -O2 Connect4/*.cpp -o "$out/connect4.exe" 2>"$log"
    rc=$?
    echo "  codigo de saida: $rc"
    if [ -s "$log" ]; then cat "$log"; else echo "  (sem avisos)"; fi
    echo "  avisos -Wall: $(grep -c 'warning:' "$log")"
    if [ $rc -ne 0 ]; then
        rc_all=1
    else
        ls -l "$out/connect4.exe"
        if command -v file >/dev/null 2>&1; then file "$out/connect4.exe"; fi
    fi
else
    echo "  x86_64-w64-mingw32-g++ nao esta instalado."
    echo "  instala com: sudo apt-get install -y g++-mingw-w64-x86-64"
    rc_all=1
fi

say "resumo"
if [ $rc_all -eq 0 ]; then echo "  testes da logica: PASS"; else echo "  testes da logica: FAIL/INCOMPLETO"; fi
exit $rc_all
