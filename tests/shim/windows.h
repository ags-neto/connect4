// ---------------------------------------------------------------------------
// Shim minimo de <windows.h> — USADO APENAS PELO BUILD DE TESTE.
//
// O jogo inclui <windows.h> em Connect4/cboard.h e usa HANDLE,
// GetStdHandle/SetConsoleTextAttribute para pintar o tabuleiro. O shim existe
// para que Connect4/cboard.cpp (a logica do tabuleiro) possa ser compilado e
// exercitado no Linux sem tocar no codigo do jogo.
//
// Fica em tests/shim/ e so entra no include path do comando de teste; nunca e
// usado para produzir o binario do jogo (esse compila-se para Windows com o
// <windows.h> verdadeiro do mingw-w64).
// ---------------------------------------------------------------------------
#ifndef C4_TEST_SHIM_WINDOWS_H
#define C4_TEST_SHIM_WINDOWS_H

typedef void *HANDLE;
typedef unsigned long DWORD;
typedef unsigned short WORD;

#define STD_OUTPUT_HANDLE ((DWORD)-11) // valor real em winbase.h

static inline HANDLE GetStdHandle(DWORD) { return (HANDLE)0; }
static inline int SetConsoleTextAttribute(HANDLE, WORD) { return 1; }

#endif // C4_TEST_SHIM_WINDOWS_H
