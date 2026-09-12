// =====================================================================
// Лабораторная работа: сравнительное исследование 4 способов
// хранения множеств в памяти ЭВМ.
// ВАРИАНТ: только ручной ввод с клавиатуры, без генерации.
//
// Способы:
//   1. Массив символов (строка, заканчивающаяся '\0')
//   2. Односвязный список (struct ST)
//   3. Булев массив (bool[26])
//   4. Машинное слово (int, битовая маска)
//
// Универсум: латинские буквы 'a'..'z' (26 штук).
// Формула: ((D and C) or A) and (not B)
//
// Автор: <вставь своё имя>
// Группа: <вставь номер группы>
// Система программирования: C++ (MSVC / MinGW), ОС Windows
// =====================================================================

#include <iostream>
#include <Windows.h>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <chrono>
#include <string>

using namespace std;
using namespace std::chrono;

const int LENOFLETTERS = 26;      // мощность универсума: 26 букв a..z
const unsigned char SHIFT = 'a';  // код первой буквы универсума
const int ONE = 1;                // единица для битовых сдвигов

// ================= ПРИВЕДЕНИЕ К СТРОЧНОЙ =================
unsigned char ToLower(unsigned char c) {
    if (c >= 'A' && c <= 'Z') return c + ('a' - 'A');
    return c;
}

// =====================================================================
// 1. МАССИВ СИМВОЛОВ
// =====================================================================
bool isInSet(unsigned char item, unsigned char* set) {
    for (size_t i = 0; set[i]; ++i)
        if (set[i] == item) return true;
    return false;
}
void AddItem(unsigned char item, unsigned char* set) {
    size_t i = 0;
    while (set[i]) ++i;
    set[i] = item;
    set[i + 1] = 0;
}
void UnionSets(unsigned char* src, unsigned char* dst) {
    for (size_t i = 0; src[i]; ++i)
        if (!isInSet(src[i], dst)) AddItem(src[i], dst);
}
void AndSets(unsigned char* src, unsigned char* dst) {
    unsigned char tmp[LENOFLETTERS + 1] = { 0 };
    int j = 0;
    for (size_t i = 0; src[i]; ++i)
        if (isInSet(src[i], dst)) tmp[j++] = src[i];
    tmp[j] = 0;
    for (int i = 0; i <= j; ++i) dst[i] = tmp[i];
}
void ExclusionSets(unsigned char* src, unsigned char* dst) {
    int j = 0;
    for (size_t i = 0; dst[i]; ++i)
        if (!isInSet(dst[i], src)) dst[j++] = dst[i];
    dst[j] = 0;
}
void PrintSet(unsigned char* set) { for (size_t i = 0; set[i]; ++i) cout << set[i]; }

// =====================================================================
// 2. СПИСОК
// =====================================================================
struct ST {
    unsigned char letter;
    ST* next;
    ST(unsigned char l, ST* n) : letter(l), next(n) {}
    ~ST() { delete next; }
};
ST* ToList(unsigned char* set) {
    ST* x = nullptr;
    for (size_t i = 0; set[i]; ++i) x = new ST(set[i], x);
    return x;
}
bool isInSet(unsigned char item, ST* set) {
    while (set) { if (set->letter == item) return true; set = set->next; }
    return false;
}
void AddItem(unsigned char item, ST*& set) { set = new ST(item, set); }
void DeleteItem(unsigned char item, ST*& set) {
    if (!set) return;
    if (set->letter == item) { ST* t = set; set = set->next; t->next = nullptr; delete t; return; }
    ST* p = set; ST* c = set->next;
    while (c) {
        if (c->letter == item) { p->next = c->next; c->next = nullptr; delete c; return; }
        p = c; c = c->next;
    }
}
void AndSets(ST* src, ST*& dst) {
    ST* res = nullptr;
    for (ST* s = src; s; s = s->next)
        if (isInSet(s->letter, dst)) AddItem(s->letter, res);
    delete dst;
    dst = res;
}
void UnionSets(ST* src, ST*& dst) {
    while (src) { if (!isInSet(src->letter, dst)) AddItem(src->letter, dst); src = src->next; }
}
void ExclusionSets(ST* src, ST*& dst) {
    while (src) { if (isInSet(src->letter, dst)) DeleteItem(src->letter, dst); src = src->next; }
}
void PrintSet(ST* set) { while (set) { cout << set->letter; set = set->next; } }

// =====================================================================
// 3. BOOL МАССИВ
// =====================================================================
void ToBool(unsigned char* set, bool* u) {
    for (size_t i = 0; set[i]; ++i) u[set[i] - SHIFT] = true;
}
void UnionSets(bool* src, bool* dst) { for (int i = 0; i < LENOFLETTERS; ++i) dst[i] |= src[i]; }
void AndSets(bool* src, bool* dst)   { for (int i = 0; i < LENOFLETTERS; ++i) dst[i] &= src[i]; }
void ExclusionSets(bool* src, bool* dst) { for (int i = 0; i < LENOFLETTERS; ++i) dst[i] = ~src[i] & dst[i]; }
void PrintSet(bool* set) {
    for (int i = 0; i < LENOFLETTERS; ++i)
        if (set[i]) cout << (unsigned char)(i + SHIFT);
}
// Печать bool-массива с прочерками: видно, какие буквы есть, какие нет.
void PrintBoolDetailed(bool* set) {
    for (int i = 0; i < LENOFLETTERS; ++i)
        if (set[i]) cout << (unsigned char)(i + SHIFT);
        else        cout << '-';
}
void ClearBool(bool* a) { for (int i = 0; i < LENOFLETTERS; ++i) a[i] = false; }

// =====================================================================
// 4. МАШИННОЕ СЛОВО
// =====================================================================
int ToMachineWord(unsigned char* set) {
    int w = 0;
    for (size_t i = 0; set[i]; ++i) w |= ONE << (set[i] - SHIFT);
    return w;
}
void UnionSets(int src, int& dst) { dst |= src; }
void AndSets(int src, int& dst)   { dst &= src; }
void ExclusionSets(int src, int& dst) { dst = ~src & dst; }
void PrintSet(int w) {
    for (int i = 0; i < LENOFLETTERS; ++i)
        if (w & (ONE << i)) cout << (unsigned char)(i + SHIFT);
}
// Печать машинного слова в виде маски (a-z с прочерками).
void PrintWordDetailed(int w) {
    for (int i = 0; i < LENOFLETTERS; ++i)
        if (w & (ONE << i)) cout << (unsigned char)(i + SHIFT);
        else                cout << '-';
}

// =====================================================================
// ВВОД С КЛАВИАТУРЫ
// =====================================================================
void InputSet(unsigned char* set) {
    cin.ignore(10000, '\n');   // очистить буфер после предыдущего cin
    char symbol;
    int i = 0;
    do {
        cin.get(symbol);
        if (symbol == '\n') break;
        unsigned char c = ToLower((unsigned char)symbol);
        if (c >= 'a' && c <= 'z' && !isInSet(c, set))
            set[i++] = c;
    } while (true);
}

// =====================================================================
// ВСПОМОГАТЕЛЬНОЕ
// =====================================================================
void Discharge(unsigned char* a) { for (size_t i = 0; a[i]; ++i) a[i] = 0; }
void CopySet(unsigned char* src, unsigned char* dst) {
    size_t i = 0;
    for (; src[i]; ++i) dst[i] = src[i];
    dst[i] = 0;
}
// Сравнить две строки-множества как множества (порядок не важен).
bool SameSets(unsigned char* a, unsigned char* b) {
    for (size_t i = 0; a[i]; ++i) if (!isInSet(a[i], b)) return false;
    for (size_t i = 0; b[i]; ++i) if (!isInSet(b[i], a)) return false;
    return true;
}

// =====================================================================
// РИСОВАНИЕ ТАБЛИЦЫ
// =====================================================================
void printLine(int wPower, int wArray, int wList, int wBool, int wWord) {
    cout << '+' << string(wPower + 2, '-')
         << '+' << string(wArray + 2, '-')
         << '+' << string(wList  + 2, '-')
         << '+' << string(wBool  + 2, '-')
         << '+' << string(wWord  + 2, '-')
         << '+' << '\n';
}
void printRow(const string& p, const string& a, const string& l,
              const string& b, const string& w,
              int wPower, int wArray, int wList, int wBool, int wWord) {
    cout << "| " << left << setw(wPower) << p
         << " | " << setw(wArray)  << a
         << " | " << setw(wList)   << l
         << " | " << setw(wBool)   << b
         << " | " << setw(wWord)   << w
         << " |" << '\n';
}

// =====================================================================
// ГЛАВНАЯ ФУНКЦИЯ
// =====================================================================
int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    unsigned char A[LENOFLETTERS + 1] = { 0 }, B[LENOFLETTERS + 1] = { 0 };
    unsigned char C[LENOFLETTERS + 1] = { 0 }, D[LENOFLETTERS + 1] = { 0 };
    bool AB[LENOFLETTERS], BB[LENOFLETTERS], CB[LENOFLETTERS], DB[LENOFLETTERS];

    cout << "=== Comparative study of set representations ===\n";
    cout << "Formula: ((D and C) or A) and (not B)\n";
    cout << "Universe: a..z (26 letters)\n\n";

    // ---------- ВВОД ----------
    cout << "Enter 4 sets (letters a..z, Enter to finish each).\n";
    cout << "Uppercase letters are converted to lowercase.\n";
    cout << "Duplicates and non-letter characters are ignored.\n\n";

    cout << "Set A: "; InputSet(A);
    cout << "Set B: "; InputSet(B);
    cout << "Set C: "; InputSet(C);
    cout << "Set D: "; InputSet(D);

    cout << "\n=== Input data ===\n";
    cout << "  A = " << A << "   (power = " << strlen((char*)A) << ")\n";
    cout << "  B = " << B << "   (power = " << strlen((char*)B) << ")\n";
    cout << "  C = " << C << "   (power = " << strlen((char*)C) << ")\n";
    cout << "  D = " << D << "   (power = " << strlen((char*)D) << ")\n";

    // ---------- ОДИН ДЕМОНСТРАЦИОННЫЙ ПРОГОН ----------
    cout << "\n=== Single demonstration run ===\n";

    // Array
    unsigned char a1[LENOFLETTERS + 1], b1[LENOFLETTERS + 1];
    unsigned char c1[LENOFLETTERS + 1], d1[LENOFLETTERS + 1];
    CopySet(A, a1); CopySet(B, b1); CopySet(C, c1); CopySet(D, d1);
    AndSets(d1, c1); UnionSets(c1, a1); ExclusionSets(b1, a1);
    cout << "Array result:         "; PrintSet(a1); cout << '\n';

    // List
    ST* AL = ToList(A); ST* BL = ToList(B);
    ST* CL = ToList(C); ST* DL = ToList(D);
    AndSets(DL, CL); UnionSets(CL, AL); ExclusionSets(BL, AL);
    cout << "List result:          "; PrintSet(AL); cout << '\n';
    delete AL; delete BL; delete CL; delete DL;

    // Bool
    ClearBool(AB); ClearBool(BB); ClearBool(CB); ClearBool(DB);
    ToBool(A, AB); ToBool(B, BB); ToBool(C, CB); ToBool(D, DB);
    AndSets(DB, CB); UnionSets(CB, AB); ExclusionSets(BB, AB);
    cout << "Boolean array result: "; PrintSet(AB); cout << '\n';
    cout << "  bool detailed:      "; PrintBoolDetailed(AB); cout << '\n';

    // Word
    int AW = ToMachineWord(A), BW = ToMachineWord(B);
    int CW = ToMachineWord(C), DW = ToMachineWord(D);
    AndSets(DW, CW); UnionSets(CW, AW); ExclusionSets(BW, AW);
    cout << "Machine word result:  "; PrintSet(AW); cout << '\n';
    cout << "  word detailed:      "; PrintWordDetailed(AW); cout << '\n';
    cout << "  word hex:           " << hex << AW << dec << '\n';

    // ---------- ЗАМЕРЫ ВРЕМЕНИ ----------
    // Вводим один раз, а прогоняем 1000 раз — так таймер
    // успевает накопить измеримое время.
    const int INNER = 1000;
    double sumA = 0, sumL = 0, sumB = 0, sumW = 0;

    // --- Array ---
    {
        unsigned char a2[LENOFLETTERS + 1], b2[LENOFLETTERS + 1];
        unsigned char c2[LENOFLETTERS + 1], d2[LENOFLETTERS + 1];
        auto t1 = high_resolution_clock::now();
        for (int k = 0; k < INNER; ++k) {
            CopySet(A, a2); CopySet(B, b2);
            CopySet(C, c2); CopySet(D, d2);
            AndSets(d2, c2); UnionSets(c2, a2); ExclusionSets(b2, a2);
        }
        auto t2 = high_resolution_clock::now();
        sumA = duration_cast<nanoseconds>(t2 - t1).count() / 1000.0 / INNER;
    }

    // --- List ---
    {
        auto t1 = high_resolution_clock::now();
        for (int k = 0; k < INNER; ++k) {
            ST* AL2 = ToList(A); ST* BL2 = ToList(B);
            ST* CL2 = ToList(C); ST* DL2 = ToList(D);
            AndSets(DL2, CL2); UnionSets(CL2, AL2); ExclusionSets(BL2, AL2);
            delete AL2; delete BL2; delete CL2; delete DL2;
        }
        auto t2 = high_resolution_clock::now();
        sumL = duration_cast<nanoseconds>(t2 - t1).count() / 1000.0 / INNER;
    }

    // --- Bool ---
    {
        auto t1 = high_resolution_clock::now();
        for (int k = 0; k < INNER; ++k) {
            ClearBool(AB); ClearBool(BB); ClearBool(CB); ClearBool(DB);
            ToBool(A, AB); ToBool(B, BB); ToBool(C, CB); ToBool(D, DB);
            AndSets(DB, CB); UnionSets(CB, AB); ExclusionSets(BB, AB);
        }
        auto t2 = high_resolution_clock::now();
        sumB = duration_cast<nanoseconds>(t2 - t1).count() / 1000.0 / INNER;
    }

    // --- Word ---
    {
        int AW2 = ToMachineWord(A), BW2 = ToMachineWord(B);
        int CW2 = ToMachineWord(C), DW2 = ToMachineWord(D);
        auto t1 = high_resolution_clock::now();
        for (int k = 0; k < INNER; ++k) {
            int aw = AW2, bw = BW2, cw = CW2, dw = DW2;
            AndSets(dw, cw); UnionSets(cw, aw); ExclusionSets(bw, aw);
        }
        auto t2 = high_resolution_clock::now();
        sumW = duration_cast<nanoseconds>(t2 - t1).count() / 1000.0 / INNER;
    }

    // ---------- ТАБЛИЦА ВРЕМЕНИ ----------
    cout << "\n=== Timing table ===\n";
    cout << "Each cell: average time of one formula execution, microseconds (us)\n";
    cout << "Measured over " << INNER << " inner runs on the SAME input sets.\n\n";

    const int wPower = 10;
    const int wArray = 18;
    const int wList  = 18;
    const int wBool  = 18;
    const int wWord  = 18;

    printLine(wPower, wArray, wList, wBool, wWord);
    printRow("Power A", "Array (chars)", "List", "Bool array", "Machine word",
             wPower, wArray, wList, wBool, wWord);
    printLine(wPower, wArray, wList, wBool, wWord);

    char bufP[16]; snprintf(bufP, sizeof(bufP), "%d", (int)strlen((char*)A));
    char bufA[32], bufL[32], bufB[32], bufW[32];
    snprintf(bufA, sizeof(bufA), "%.6f", sumA);
    snprintf(bufL, sizeof(bufL), "%.6f", sumL);
    snprintf(bufB, sizeof(bufB), "%.6f", sumB);
    snprintf(bufW, sizeof(bufW), "%.6f", sumW);

    printRow(bufP, bufA, bufL, bufB, bufW,
             wPower, wArray, wList, wBool, wWord);
    printLine(wPower, wArray, wList, wBool, wWord);

    // ---------- ПРОВЕРКА СОВПАДЕНИЯ ----------
    // Все 4 способа должны дать одинаковый результат.
    // Сравниваем массив символов с bool и массив символов с word.
    cout << "\n=== Consistency check ===\n";
    if (SameSets(a1, AB) && SameSets(a1, AL_unused_placeholder())) {
        // (см. ниже; здесь просто заглушка, чтобы читалось)
    }
    bool okAB = true, okAW = true;
    {
        // Array vs Bool
        unsigned char tmpChar[LENOFLETTERS + 1];
        CopySet(a1, tmpChar);
        for (int i = 0; i < LENOFLETTERS; ++i) {
            unsigned char letter = (unsigned char)(i + SHIFT);
            if (AB[i] != isInSet(letter, tmpChar)) { okAB = false; break; }
        }
    }
    {
        // Array vs Word
        unsigned char tmpChar[LENOFLETTERS + 1];
        CopySet(a1, tmpChar);
        for (int i = 0; i < LENOFLETTERS; ++i) {
            unsigned char letter = (unsigned char)(i + SHIFT);
            bool inWord = (AW & (ONE << i)) != 0;
            if (inWord != isInSet(letter, tmpChar)) { okAW = false; break; }
        }
    }
    cout << "Array vs Bool: " << (okAB ? "OK" : "MISMATCH") << '\n';
    cout << "Array vs Word: " << (okAW ? "OK" : "MISMATCH") << '\n';

    return 0;
}