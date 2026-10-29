#include <stdio.h>
#include <ctype.h>
#include <string.h>

// ============================================================
// Обёртка вокруг char[27] — чтобы возвращать множество из функции
// по значению и писать формулу одной строкой.
// 26 букв + '\0' всегда хватает: буквы в множестве уникальны.
// ============================================================
typedef struct {
    char d[27];
} Buf;

// ============================================================
// 1. СОРТИРОВКА МНОЖЕСТВА — O(n)
//    Элементы уникальны, поэтому счётчики не нужны — только флаги.
// ============================================================
Buf sortSet(const char* s) {
    char present[26] = {0};        // 0 = буквы нет, 1 = есть
    Buf out;

    // 1-й проход: помечаем присутствие букв
    for (int i = 0; s[i] != '\0'; ++i) {
        char c = (char)tolower((unsigned char)s[i]);
        if (c >= 'a' && c <= 'z')
            present[c - 'a'] = 1;
    }

    // 2-й проход: выписываем буквы по алфавиту
    int k = 0;
    for (int c = 0; c < 26; ++c)
        if (present[c])
            out.d[k++] = (char)('a' + c);

    out.d[k] = '\0';
    return out;
}

// ============================================================
// 2. AND — пересечение
// ============================================================
Buf setAnd(const char* A, const char* B) {
    Buf out;
    int i = 0, j = 0, k = 0;

    while (A[i] != '\0' && B[j] != '\0') {
        if      (A[i] == B[j]) { out.d[k++] = A[i]; ++i; ++j; }
        else if (A[i] <  B[j]) { ++i; }   // A[i] нет в B
        else                   { ++j; }   // B[j] нет в A
    }

    out.d[k] = '\0';
    return out;
}

// ============================================================
// 3. OR — объединение
// ============================================================
Buf setOr(const char* A, const char* B) {
    Buf out;
    int i = 0, j = 0, k = 0;

    while (A[i] != '\0' && B[j] != '\0') {
        if      (A[i] == B[j]) { out.d[k++] = A[i]; ++i; ++j; }
        else if (A[i] <  B[j]) { out.d[k++] = A[i]; ++i; }
        else                   { out.d[k++] = B[j]; ++j; }
    }
    while (A[i] != '\0') out.d[k++] = A[i++];   // хвост A
    while (B[j] != '\0') out.d[k++] = B[j++];   // хвост B

    out.d[k] = '\0';
    return out;
}

// ============================================================
// 4. NOT — дополнение относительно 'a'..'z'
// ============================================================
Buf setNot(const char* A) {
    Buf out;
    int i = 0, k = 0;

    for (char c = 'a'; c <= 'z'; ++c) {
        while (A[i] != '\0' && A[i] < c) ++i;
        if (A[i] == c) ++i;      // буква есть в A → пропускаем
        else out.d[k++] = c;     // буквы нет → добавляем
    }

    out.d[k] = '\0';
    return out;
}

// ============================================================
// Печать множества
// ============================================================
void printSet(const char* name, const char* s) {
    printf("%s = { ", name);
    for (int i = 0; s[i] != '\0'; ++i) printf("%c ", s[i]);
    printf("}\n");
}

// ============================================================
// MAIN
// ============================================================
int main(void) {
    char inputA[1000], inputB[1000], inputC[1000], inputD[1000];

    printf("Enter set A: "); fgets(inputA, sizeof(inputA), stdin);
    printf("Enter set B: "); fgets(inputB, sizeof(inputB), stdin);
    printf("Enter set C: "); fgets(inputC, sizeof(inputC), stdin);
    printf("Enter set D: "); fgets(inputD, sizeof(inputD), stdin);

    // Убираем '\n' в конце, который добавляет fgets
    inputA[strcspn(inputA, "\n")] = '\0';
    inputB[strcspn(inputB, "\n")] = '\0';
    inputC[strcspn(inputC, "\n")] = '\0';
    inputD[strcspn(inputD, "\n")] = '\0';

    // Отсортированные множества
    Buf A = sortSet(inputA);
    Buf B = sortSet(inputB);
    Buf C = sortSet(inputC);
    Buf D = sortSet(inputD);

    printSet("A", A.d);
    printSet("B", B.d);
    printSet("C", C.d);
    printSet("D", D.d);

    // ------------------------------------------------------------
    // ФОРМУЛА В ОДНУ СТРОКУ.
    // Правило: результат setAnd/setOr/setNot — это Buf.
    //          Если он идёт на вход другой функции — пишем .d.
    // Пример: ((D AND C) OR A) AND NOT B
    // ------------------------------------------------------------
    // E = (A \ B) ∪ (C ∩ D) = (A ∩ NOT(B)) ∪ (C ∩ D)
Buf result = setOr(
    setAnd(A.d, setNot(B.d).d).d,
    setAnd(C.d, D.d).d
);

    printf("\n--- Result ---\n");
    printSet("11 variant:", result.d);
    printf("Result size: %zu\n", strlen(result.d));

    return 0;
}