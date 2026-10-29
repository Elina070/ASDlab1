// =============================================================
// Лабораторная работа №1: сравнительное исследование 4 способов
// хранения множеств в памяти ЭВМ.
//
// ВАРИАНТ 11.
// Универсум: латинские буквы (строчные 'a'...'z', 26 штук).
//            Заглавные буквы при вводе приводятся к строчным.
// Формула:   E = (A \ B) ∪ (C ∩ D)
//
// Способы:
//   1. Массив символов (строка, заканчивающаяся '\0')
//   2. Односвязный список (struct ST)
//   3. Булев массив (bool[26])
//   4. Машинное слово (int, битовая маска)
//
// Система программирования: C++ (MSVC / MinGW), ОС Windows
// =============================================================

#include <iostream>
#include <Windows.h>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <chrono>
#include <cstring>
#include <string>

using namespace std;
using namespace std::chrono;

const int LENOFLETTERS = 26;      // мощность универсума (буквы a..z)
const unsigned char SHIFT = 'a';  // код буквы 'a'
const int ONE = 1;

// Глобальная volatile-переменная для предотвращения оптимизации (Dead Code Elimination).
volatile int g_sink = 0;

// =============================================================
// ПРИВЕДЕНИЕ РЕГИСТРА И ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ
// =============================================================

unsigned char ToLower(unsigned char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A' + 'a';
    return c;
}

void Normalize(unsigned char* set) {
    for (size_t i = 0; set[i]; ++i)
        set[i] = ToLower(set[i]);
}

void Discharge(unsigned char* a) {
    for (size_t i = 0; i <= LENOFLETTERS; ++i) a[i] = 0;
}

// =============================================================
// 1. МАССИВ СИМВОЛОВ (Неразрушающие операции)
// =============================================================

bool isInSet(unsigned char item, const unsigned char* set) {
    for (size_t i = 0; set[i]; ++i)
        if (set[i] == item) return true;
    return false;
}

void AddItem(unsigned char item, unsigned char* set) {
    if (isInSet(item, set)) return;
    size_t i = 0;
    while (set[i]) ++i;
    set[i] = item;
    set[i + 1] = 0;
}

// dst = A \ B
void ExclusionSets(const unsigned char* A, const unsigned char* B, unsigned char* dst) {
    size_t j = 0;
    for (size_t i = 0; A[i]; ++i)
        if (!isInSet(A[i], B)) dst[j++] = A[i];
    dst[j] = 0;
}

// dst = C ∩ D
void AndSets(const unsigned char* C, const unsigned char* D, unsigned char* dst) {
    size_t j = 0;
    for (size_t i = 0; C[i]; ++i)
        if (isInSet(C[i], D)) dst[j++] = C[i];
    dst[j] = 0;
}

// dst = T1 ∪ T2
void UnionSets(const unsigned char* T1, const unsigned char* T2, unsigned char* dst) {
    size_t j = 0;
    for (; T1[j]; ++j) dst[j] = T1[j];
    for (size_t i = 0; T2[i]; ++i) {
        if (!isInSet(T2[i], dst)) dst[j++] = T2[i];
    }
    dst[j] = 0;
}

void PrintSet(const unsigned char* set) {
    for (size_t i = 0; set[i]; ++i) cout << set[i];
}

// =============================================================
// 2. ОДНОСВЯЗНЫЙ СПИСОК (Неразрушающие операции)
// =============================================================

struct ST {
    unsigned char letter;
    ST* next;
    ST(unsigned char l, ST* n = nullptr) : letter(l), next(n) {}
};

void FreeList(ST* head) {
    while (head) {
        ST* temp = head;
        head = head->next;
        delete temp;
    }
}

ST* ToList(const unsigned char* set) {
    ST* head = nullptr;
    for (size_t i = 0; set[i]; ++i) head = new ST(set[i], head);
    return head;
}

bool isInSet(unsigned char item, const ST* set) {
    while (set) {
        if (set->letter == item) return true;
        set = set->next;
    }
    return false;
}

// res = A \ B
ST* ExclusionSets(const ST* A, const ST* B) {
    ST* res = nullptr;
    for (const ST* a = A; a; a = a->next)
        if (!isInSet(a->letter, B)) res = new ST(a->letter, res);
    return res;
}

// res = C ∩ D
ST* AndSets(const ST* C, const ST* D) {
    ST* res = nullptr;
    for (const ST* c = C; c; c = c->next)
        if (isInSet(c->letter, D)) res = new ST(c->letter, res);
    return res;
}

// res = T1 ∪ T2
ST* UnionSets(const ST* T1, const ST* T2) {
    ST* res = nullptr;
    for (const ST* t1 = T1; t1; t1 = t1->next)
        res = new ST(t1->letter, res);
    for (const ST* t2 = T2; t2; t2 = t2->next)
        if (!isInSet(t2->letter, res)) res = new ST(t2->letter, res);
    return res;
}

void PrintSet(const ST* set) {
    for (const ST* s = set; s; s = s->next) cout << s->letter;
}

// =============================================================
// 3. БУЛЕВ МАССИВ (Неразрушающие операции)
// =============================================================

void ToBool(const unsigned char* set, bool* b) {
    for (int i = 0; i < LENOFLETTERS; ++i) b[i] = false;
    for (size_t i = 0; set[i]; ++i)
        b[set[i] - SHIFT] = true;
}

void ExclusionSets(const bool* A, const bool* B, bool* dst) {
    for (int i = 0; i < LENOFLETTERS; ++i) dst[i] = A[i] && !B[i];
}

void AndSets(const bool* C, const bool* D, bool* dst) {
    for (int i = 0; i < LENOFLETTERS; ++i) dst[i] = C[i] && D[i];
}

void UnionSets(const bool* T1, const bool* T2, bool* dst) {
    for (int i = 0; i < LENOFLETTERS; ++i) dst[i] = T1[i] || T2[i];
}

void PrintSet(const bool* b) {
    for (int i = 0; i < LENOFLETTERS; ++i)
        if (b[i]) cout << (unsigned char)(i + SHIFT);
}

// =============================================================
// 4. МАШИННОЕ СЛОВО (Битовая маска)
// =============================================================

int ToMachineWord(const unsigned char* set) {
    int w = 0;
    for (size_t i = 0; set[i]; ++i)
        w |= ONE << (set[i] - SHIFT);
    return w;
}

int ExclusionSets(int A, int B) { return A & ~B; }
int AndSets(int C, int D) { return C & D; }
int UnionSets(int T1, int T2) { return T1 | T2; }

void PrintSet(int w) {
    for (int i = 0; i < LENOFLETTERS; ++i)
        if (w & (ONE << i)) cout << (unsigned char)(i + SHIFT);
}

// =============================================================
// ГЕНЕРАТОР И ВЫВОД ТАБЛИЦЫ
// =============================================================

void Generator(int len, unsigned char* arr) {
    unsigned char u[LENOFLETTERS];
    for (int i = 0; i < LENOFLETTERS; ++i) u[i] = SHIFT + i;
    for (int i = 0; i < len; ++i) {
        int p = rand() % (LENOFLETTERS - i);
        if (p) swap(u[p + i], u[i]);
    }
    for (int i = 0; i < len; ++i) AddItem(u[i], arr);
}

void printLine(int wPower, int wArray, int wList, int wBool, int wWord) {
    cout << '+' << string(wPower + 2, '-')
         << '+' << string(wArray + 2, '-')
         << '+' << string(wList + 2, '-')
         << '+' << string(wBool + 2, '-')
         << '+' << string(wWord + 2, '-')
         << '+' << '\n';
}

void printRow(const string& p, const string& a, const string& l,
              const string& b, const string& w,
              int wPower, int wArray, int wList, int wBool, int wWord) {
    cout << "| " << left << setw(wPower) << p
         << "| " << setw(wArray) << a
         << "| " << setw(wList) << l
         << "| " << setw(wBool) << b
         << "| " << setw(wWord) << w
         << "| " << '\n';
}

// =============================================================
// ГЛАВНАЯ ФУНКЦИЯ И ИЗМЕРЕНИЯ
// =============================================================

int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    srand(static_cast<unsigned int>(time(0)));

    unsigned char A[LENOFLETTERS + 1] = { 0 }, B[LENOFLETTERS + 1] = { 0 };
    unsigned char C[LENOFLETTERS + 1] = { 0 }, D[LENOFLETTERS + 1] = { 0 };

    cout << "=== Comparative study of set representations ===\n";
    cout << "Variant 11. Formula: E = (A \\ B) U (C n D)\n";
    cout << "Universe: Latin letters (lowercase a..z, 26 letters)\n\n";

    const int INNER_SLOW = 100000;
    const int INNER_FAST = 50000000;

    cout << "=== Timing table ===\n";
    cout << "Each cell: average time of one formula execution, microseconds (us)\n\n";

    const int wPower = 7;
    const int wArray = 18;
    const int wList  = 18;
    const int wBool  = 18;
    const int wWord  = 18;

    printLine(wPower, wArray, wList, wBool, wWord);
    printRow("Power", "Array (chars)", "List", "Bool array", "Machine word",
             wPower, wArray, wList, wBool, wWord);
    printLine(wPower, wArray, wList, wBool, wWord);

    for (int len = 2; len <= LENOFLETTERS; len += 2) {
        Discharge(A); Discharge(B); Discharge(C); Discharge(D);
        Generator(len, A); Generator(len, B);
        Generator(len, C); Generator(len, D);

        // ---------------------------------------------------------
        // 1. ARRAY (Замер ТОЛЬКО вычисления формулы)
        // ---------------------------------------------------------
        double sumA = 0;
        {
            unsigned char res1[LENOFLETTERS + 1], res2[LENOFLETTERS + 1], res[LENOFLETTERS + 1];
            auto t1 = high_resolution_clock::now();
            for (int k = 0; k < INNER_SLOW; ++k) {
                ExclusionSets(A, B, res1);   // res1 = A \ B
                AndSets(C, D, res2);         // res2 = C ∩ D
                UnionSets(res1, res2, res);  // res  = res1 ∪ res2
            }
            auto t2 = high_resolution_clock::now();
            g_sink += res[0]; // Защита от DCE
            sumA = duration_cast<nanoseconds>(t2 - t1).count() / 1000.0 / INNER_SLOW;
        }

        // ---------------------------------------------------------
        // 2. LIST (Подготовка структуры ДО замера времени)
        // ---------------------------------------------------------
        ST* AL = ToList(A); ST* BL = ToList(B);
        ST* CL = ToList(C); ST* DL = ToList(D);
        double sumL = 0;
        {
            auto t1 = high_resolution_clock::now();
            for (int k = 0; k < INNER_SLOW; ++k) {
                ST* res1 = ExclusionSets(AL, BL); // A \ B
                ST* res2 = AndSets(CL, DL);       // C ∩ D
                ST* res  = UnionSets(res1, res2); // res1 ∪ res2

                g_sink += res ? res->letter : 0;  // Защита от DCE

                // Удаление ВРЕМЕННЫХ промежуточных результатов выполнения операции
                FreeList(res1); FreeList(res2); FreeList(res);
            }
            auto t2 = high_resolution_clock::now();
            sumL = duration_cast<nanoseconds>(t2 - t1).count() / 1000.0 / INNER_SLOW;
        }
        // Очистка исходных списков ПОСЛЕ замера
        FreeList(AL); FreeList(BL); FreeList(CL); FreeList(DL);

        // ---------------------------------------------------------
        // 3. BOOL (Подготовка структуры ДО замера времени)
        // ---------------------------------------------------------
        bool AB[LENOFLETTERS], BB[LENOFLETTERS], CB[LENOFLETTERS], DB[LENOFLETTERS];
        ToBool(A, AB); ToBool(B, BB); ToBool(C, CB); ToBool(D, DB);
        double sumB = 0;
        {
            bool res1[LENOFLETTERS], res2[LENOFLETTERS], res[LENOFLETTERS];
            auto t1 = high_resolution_clock::now();
            for (int k = 0; k < INNER_FAST; ++k) {
                ExclusionSets(AB, BB, res1);  // A \ B
                AndSets(CB, DB, res2);        // C ∩ D
                UnionSets(res1, res2, res);   // res1 ∪ res2
            }
            auto t2 = high_resolution_clock::now();
            g_sink += res[0]; // Защита от DCE
            sumB = duration_cast<nanoseconds>(t2 - t1).count() / 1000.0 / INNER_FAST;
        }

        // ---------------------------------------------------------
        // 4. MACHINE WORD (Подготовка структуры ДО замера времени)
        // ---------------------------------------------------------
        int AW = ToMachineWord(A), BW = ToMachineWord(B);
        int CW = ToMachineWord(C), DW = ToMachineWord(D);
        double sumW = 0;
        {
            int res = 0;
            auto t1 = high_resolution_clock::now();
            for (int k = 0; k < INNER_FAST; ++k) {
                int res1 = ExclusionSets(AW, BW); // A \ B
                int res2 = AndSets(CW, DW);       // C ∩ D
                res = UnionSets(res1, res2);      // res1 ∪ res2
            }
            auto t2 = high_resolution_clock::now();
            g_sink += res; // Защита от DCE
            sumW = duration_cast<nanoseconds>(t2 - t1).count() / 1000.0 / INNER_FAST;
        }

        char bufP[16], bufA[32], bufL[32], bufB[32], bufW[32];
        snprintf(bufP, sizeof(bufP), "%d", len);
        snprintf(bufA, sizeof(bufA), "%.6f", sumA);
        snprintf(bufL, sizeof(bufL), "%.6f", sumL);
        snprintf(bufB, sizeof(bufB), "%.6f", sumB);
        snprintf(bufW, sizeof(bufW), "%.6f", sumW);
        printRow(bufP, bufA, bufL, bufB, bufW,
                 wPower, wArray, wList, wBool, wWord);
    }
    printLine(wPower, wArray, wList, wBool, wWord);

    // =============================================================
    // ДЕМОНСТРАЦИОННЫЙ ПРОГОН (Проверка правильности вычислений)
    // =============================================================
    cout << "\n=== Single demonstration run ===\n";
    Discharge(A); Discharge(B); Discharge(C); Discharge(D);
    Generator(10, A); Generator(10, B);
    Generator(10, C); Generator(10, D);
    Normalize(A); Normalize(B); Normalize(C); Normalize(D);

    cout << "Input data:\n";
    cout << " A = "; PrintSet(A); cout << '\n';
    cout << " B = "; PrintSet(B); cout << '\n';
    cout << " C = "; PrintSet(C); cout << '\n';
    cout << " D = "; PrintSet(D); cout << '\n';

    // Array demo
    {
        unsigned char r1[LENOFLETTERS + 1], r2[LENOFLETTERS + 1], res[LENOFLETTERS + 1];
        ExclusionSets(A, B, r1);
        AndSets(C, D, r2);
        UnionSets(r1, r2, res);
        cout << "Array result:         "; PrintSet(res); cout << '\n';
    }

    // List demo
    {
        ST* AL = ToList(A); ST* BL = ToList(B);
        ST* CL = ToList(C); ST* DL = ToList(D);
        ST* r1 = ExclusionSets(AL, BL);
        ST* r2 = AndSets(CL, DL);
        ST* res = UnionSets(r1, r2);
        cout << "List result:          "; PrintSet(res); cout << '\n';
        FreeList(AL); FreeList(BL); FreeList(CL); FreeList(DL);
        FreeList(r1); FreeList(r2); FreeList(res);
    }

    // Bool demo
    {
        bool AB[LENOFLETTERS], BB[LENOFLETTERS], CB[LENOFLETTERS], DB[LENOFLETTERS];
        bool r1[LENOFLETTERS], r2[LENOFLETTERS], res[LENOFLETTERS];
        ToBool(A, AB); ToBool(B, BB); ToBool(C, CB); ToBool(D, DB);
        ExclusionSets(AB, BB, r1);
        AndSets(CB, DB, r2);
        UnionSets(r1, r2, res);
        cout << "Boolean array result: "; PrintSet(res); cout << '\n';
    }

    // Word demo
    {
        int AW = ToMachineWord(A), BW = ToMachineWord(B);
        int CW = ToMachineWord(C), DW = ToMachineWord(D);
        int r1 = ExclusionSets(AW, BW);
        int r2 = AndSets(CW, DW);
        int res = UnionSets(r1, r2);
        cout << "Machine word result:  "; PrintSet(res); cout << '\n';
    }

    return 0;
}