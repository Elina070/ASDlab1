#include <iostream>
#include <Windows.h>

using namespace std;

const int LENOFLETTERS = 26;
const unsigned char SHIFT = 'a';
const int ONE = 1;

// ================= HELPERS =================
unsigned char ToLower(unsigned char c) {
    if (c >= 'A' && c <= 'Z') return c + ('a' - 'A');
    return c;
}

// ================= CHARACTER ARRAY =================
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

// ================= LINKED LIST =================
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

// ================= BOOL ARRAY =================
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
void ClearBool(bool* a) { for (int i = 0; i < LENOFLETTERS; ++i) a[i] = false; }

// ================= MACHINE WORD =================
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

// ================= HELPERS =================
void CopySet(unsigned char* src, unsigned char* dst) {
    size_t i = 0;
    for (; src[i]; ++i) dst[i] = src[i];
    dst[i] = 0;
}

// Reading one set from keyboard: letters a..z (uppercase allowed), Enter to finish
void InputSet(unsigned char* set) {
    char symbol;
    int i = 0;
    while (cin.get(symbol)) {
        if (symbol == '\n') break;
        unsigned char c = ToLower((unsigned char)symbol);
        if (c >= 'a' && c <= 'z' && !isInSet(c, set))
            set[i++] = c;
    }
}

// ================= MAIN =================
int main() {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    unsigned char A[LENOFLETTERS + 1] = { 0 }, B[LENOFLETTERS + 1] = { 0 };
    unsigned char C[LENOFLETTERS + 1] = { 0 }, D[LENOFLETTERS + 1] = { 0 };
    bool AB[LENOFLETTERS], BB[LENOFLETTERS], CB[LENOFLETTERS], DB[LENOFLETTERS];

    cout << "=== Comparative study of set representations ===\n";
    cout << "Formula: ((D and C) or A) and (not B)\n";
    cout << "Universe: a..z\n\n";

    cout << "Enter sets A, B, C, D (letters a..z, Enter to finish each).\n";
    cout << "A: "; InputSet(A);
    cout << "B: "; InputSet(B);
    cout << "C: "; InputSet(C);
    cout << "D: "; InputSet(D);

    cout << "\n=== Input data ===\n";
    cout << "A = " << A << '\n';
    cout << "B = " << B << '\n';
    cout << "C = " << C << '\n';
    cout << "D = " << D << '\n';

    // ---------- Array ----------
    unsigned char a1[LENOFLETTERS + 1], b1[LENOFLETTERS + 1];
    unsigned char c1[LENOFLETTERS + 1], d1[LENOFLETTERS + 1];
    CopySet(A, a1); CopySet(B, b1); CopySet(C, c1); CopySet(D, d1);
    AndSets(d1, c1); UnionSets(c1, a1); ExclusionSets(b1, a1);
    cout << "Array result:         "; PrintSet(a1); cout << '\n';

    // ---------- List ----------
    ST* AL = ToList(A); ST* BL = ToList(B);
    ST* CL = ToList(C); ST* DL = ToList(D);
    AndSets(DL, CL); UnionSets(CL, AL); ExclusionSets(BL, AL);
    cout << "List result:          "; PrintSet(AL); cout << '\n';
    delete AL; delete BL; delete CL; delete DL;

    // ---------- Bool ----------
    ClearBool(AB); ClearBool(BB); ClearBool(CB); ClearBool(DB);
    ToBool(A, AB); ToBool(B, BB); ToBool(C, CB); ToBool(D, DB);
    AndSets(DB, CB); UnionSets(CB, AB); ExclusionSets(BB, AB);
    cout << "Boolean array result: "; PrintSet(AB); cout << '\n';

    // ---------- Word ----------
    int AW = ToMachineWord(A), BW = ToMachineWord(B);
    int CW = ToMachineWord(C), DW = ToMachineWord(D);
    AndSets(DW, CW); UnionSets(CW, AW); ExclusionSets(BW, AW);
    cout << "Machine word result:  "; PrintSet(AW); cout << '\n';

    return 0;
}