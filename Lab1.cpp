// =====================================================================
// Лабораторная работа: сравнительное исследование 4 способов
// хранения множеств в памяти ЭВМ.
//
// Способы:
//   1. Массив символов (строка, заканчивающаяся '\0')
//   2. Односвязный список (struct ST)
//   3. Булев массив (bool[26])
//   4. Машинное слово (int, битовая маска)
//
// Универсум: латинские буквы 'a'..'z' (26 штук).
// Формула: ((D and C) or A) and (not B) 
// Система программирования: C++ (MSVC / MinGW), ОС Windows 11
// =====================================================================

// ---- ПОДКЛЮЧЕНИЕ БИБЛИОТЕК ----
// <Windows.h> — для SetConsoleCP/SetConsoleOutputCP (кодировка консоли).
// <chrono>    — таймер с наносекундной точностью (в C такого нет).
#include <iostream>
#include <Windows.h>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <chrono>
#include <string> //только для рисования таблицы, не для работы с множествами

using namespace std;
using namespace std::chrono;


// ================= КОНСТАНТЫ =================
const int LENOFLETTERS = 26;      // мощность универсума: 26 букв a..z
const unsigned char SHIFT = 'a';  // код первой буквы универсума
const int ONE = 1;                // единица для битовых сдвигов

// =====================================================================
// 1. МАССИВ СИМВОЛОВ
// Множество хранится как строка: буквы подряд, в конце '\0'.
// Одна и та же функция isInSet используется и для массива, и для строки —
// но с РАЗНЫМИ вторыми параметрами (перегрузка функций в C++).
// =====================================================================

// Проверка, есть ли элемент item в множестве set (строка).
bool isInSet(unsigned char item, unsigned char* set) {
    // Идём по строке, пока не встретим '\0' (конец строки).
    for (size_t i = 0; set[i]; ++i)
        if (set[i] == item) return true;  
    return false;                          
}

// Добавить элемент в конец множества-строки.
void AddItem(unsigned char item, unsigned char* set) {
    size_t i = 0;
    while (set[i]) ++i;   
    set[i] = item;        
    set[i + 1] = 0;       
}

// Объединение: все элементы src, которых нет в dst, добавляем в dst.
// src не меняется, dst меняется.
void UnionSets(unsigned char* src, unsigned char* dst) {
    for (size_t i = 0; src[i]; ++i)
        if (!isInSet(src[i], dst)) AddItem(src[i], dst);
}

// Пересечение: dst = src ∩ dst.
// Во временный буфер tmp собираем общие элементы, потом копируем в dst.
// ВАЖНО: здесь dst — это и вход, и выход, поэтому без tmp не обойтись:
// нельзя писать в dst во время чтения из него же.
void AndSets(unsigned char* src, unsigned char* dst) {
    unsigned char tmp[LENOFLETTERS + 1] = { 0 };  // буфер
    int j = 0;
    for (size_t i = 0; src[i]; ++i)
        if (isInSet(src[i], dst)) tmp[j++] = src[i];  
    tmp[j] = 0;                                       
    for (int i = 0; i <= j; ++i) dst[i] = tmp[i];     
}

// Разность: dst = dst \ src.
// Идём по dst, оставляем только те, которых НЕТ в src.
// Здесь можно писать прямо в dst, потому что j <= i всегда:
void ExclusionSets(unsigned char* src, unsigned char* dst) {
    int j = 0;
    for (size_t i = 0; dst[i]; ++i)
        if (!isInSet(dst[i], src)) dst[j++] = dst[i];
    dst[j] = 0;   
}

// Печать множества-строки. В C++ cout работает как printf
void PrintSet(unsigned char* set) {
    for (size_t i = 0; set[i]; ++i) cout << set[i];
}

// =====================================================================
// 2. ОДНОСВЯЗНЫЙ СПИСОК
// struct ST — это аналог struct в C. Отличие: в C++ внутри struct
// можно объявить конструктор ST(...) и деструктор ~ST().
// Конструктор вызывается автоматически при new ST(...),
// деструктор — при delete.
// =====================================================================

struct ST {
    unsigned char letter;   // буква
    ST* next;               // указатель на следующий узел

    // Конструктор: вызывается new ST(l, n).
    ST(unsigned char l, ST* n) : letter(l), next(n) {}

    // Деструктор: вызывается delete. Рекурсивно удаляет весь хвост.
    ~ST() { delete next; }
};

// Построить список из строки-множества.
// Элементы добавляются в НАЧАЛО, поэтому порядок в списке обратный.
ST* ToList(unsigned char* set) {
    ST* x = nullptr;  
    for (size_t i = 0; set[i]; ++i) x = new ST(set[i], x);
    return x;
}

// Проверка, есть ли элемент в списке. Это ПЕРЕГРУЗКА функции isInSet —
// компилятор сам выберет версию по типу второго аргумента (ST* или char*).
bool isInSet(unsigned char item, ST* set) {
    while (set) {
        if (set->letter == item) return true;
        set = set->next;
    }
    return false;
}

// Добавить элемент в начало списка (O(1)).
void AddItem(unsigned char item, ST*& set) {
    // ST*& set — ссылка на указатель. В C аналог — ST** set,
    // но пришлось бы писать (*set) везде. Ссылки в C++ удобнее.
    set = new ST(item, set);
}

// Удалить элемент из списка (первое вхождение).
// Тонкость: если удаляем голову, надо поменять сам указатель set,
// поэтому он передаётся по ссылке ST*&.
void DeleteItem(unsigned char item, ST*& set) {
    if (!set) return;   // пустой список — ничего не делаем
    if (set->letter == item) {
        ST* t = set;
        set = set->next;
        t->next = nullptr;   // отрезаем хвост, чтобы деструктор не удалил всё
        delete t;
        return;
    }
    ST* p = set;          // предыдущий
    ST* c = set->next;    // текущий
    while (c) {
        if (c->letter == item) {
            p->next = c->next;
            c->next = nullptr;
            delete c;
            return;
        }
        p = c;
        c = c->next;
    }
}

// Пересечение: dst = src ∩ dst.
// Собираем результат в новый список res, потом подменяем dst.
// ВАЖНО: сначала delete dst, потом присваивание — иначе утечка.
void AndSets(ST* src, ST*& dst) {
    ST* res = nullptr;
    for (ST* s = src; s; s = s->next)
        if (isInSet(s->letter, dst)) AddItem(s->letter, res);
    delete dst;      // удаляем старый список
    dst = res;       // подменяем новым
}

// Объединение: dst = dst ∪ src.
void UnionSets(ST* src, ST*& dst) {
    while (src) {
        if (!isInSet(src->letter, dst)) AddItem(src->letter, dst);
        src = src->next;
    }
}

// Разность: dst = dst \ src.
void ExclusionSets(ST* src, ST*& dst) {
    while (src) {
        if (isInSet(src->letter, dst)) DeleteItem(src->letter, dst);
        src = src->next;
    }
}

// Печать списка.
void PrintSet(ST* set) {
    while (set) { cout << set->letter; set = set->next; }
}

// =====================================================================
// 3. БУЛЕВ МАССИВ
// bool[26] — для каждой буквы свой флаг. Буква 'a' → индекс 0, ..., 'z' → 25.
// Размер фиксирован, поэтому все операции — O(26) = O(1).
// =====================================================================

// Заполнить булев массив по строке-множеству.
void ToBool(unsigned char* set, bool* u) {
    for (size_t i = 0; set[i]; ++i) u[set[i] - SHIFT] = true;
    // set[i] - SHIFT даёт номер буквы: 'a' - 'a' = 0, 'z' - 'a' = 25.
}

// Объединение: побитовое ИЛИ для булевых значений.
void UnionSets(bool* src, bool* dst) {
    for (int i = 0; i < LENOFLETTERS; ++i) dst[i] |= src[i];
}

// Пересечение: побитовое И.
void AndSets(bool* src, bool* dst) {
    for (int i = 0; i < LENOFLETTERS; ++i) dst[i] &= src[i];
}

// Разность: dst = dst И НЕ src. Формула: dst[i] = ~src[i] & dst[i].
void ExclusionSets(bool* src, bool* dst) {
    for (int i = 0; i < LENOFLETTERS; ++i) dst[i] = ~src[i] & dst[i];
}

// Печать: выводим буквы, для которых флаг true.
void PrintSet(bool* set) {
    for (int i = 0; i < LENOFLETTERS; ++i)
        if (set[i]) cout << (unsigned char)(i + SHIFT);
    // (unsigned char) — явное приведение int → char, чтобы cout
    // напечатал символ, а не число.
}

// Очистить булев массив (обнулить все флаги).
void ClearBool(bool* a) {
    for (int i = 0; i < LENOFLETTERS; ++i) a[i] = false;
}

// =====================================================================
// 4. МАШИННОЕ СЛОВО
// Множество — это int, где i-й бит = 1, если буква ('a'+i) в множестве.
// Все операции — одна машинная инструкция над int.
// =====================================================================

// Преобразовать строку-множество в битовую маску.
int ToMachineWord(unsigned char* set) {
    int w = 0;
    for (size_t i = 0; set[i]; ++i) w |= ONE << (set[i] - SHIFT);
    // ONE << k — сдвиг единицы на k позиций: получаем бит с номером k.
    // |= — накопление битов в маске.
    return w;
}

// Объединение — побитовое ИЛИ.
void UnionSets(int src, int& dst) { dst |= src; }

// Пересечение — побитовое И.
void AndSets(int src, int& dst) { dst &= src; }

// Разность — dst &= ~src.
void ExclusionSets(int src, int& dst) { dst = ~src & dst; }

// Печать: проверяем i-й бит, если 1 — выводим букву.
void PrintSet(int w) {
    for (int i = 0; i < LENOFLETTERS; ++i)
        if (w & (ONE << i)) cout << (unsigned char)(i + SHIFT);
}

// =====================================================================
// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ
// =====================================================================

// Сгенерировать случайное подмножество мощности len из универсума a..z.
// Используется алгоритм Фишера–Йетса (частичное перемешивание).
void Generator(int len, unsigned char* arr) {
    unsigned char u[LENOFLETTERS];
    for (int i = 0; i < LENOFLETTERS; ++i) u[i] = SHIFT + i;  // u = a..z

    // Перемешиваем первые len элементов с оставшимися.
    for (int i = 0; i < len; ++i) {
        int p = rand() % (LENOFLETTERS - i);   // случайный индекс [0, 26-i)
        if (p) swap(u[p + i], u[i]);            // swap — из <algorithm>,
    
    }
    // Первые len букв массива u — случайное подмножество мощности len.
    for (int i = 0; i < len; ++i) AddItem(u[i], arr);
}

// Обнулить множество-строку.
void Discharge(unsigned char* a) {
    for (size_t i = 0; a[i]; ++i) a[i] = 0;
}

// Скопировать множество-строку из src в dst.
void CopySet(unsigned char* src, unsigned char* dst) {
    size_t i = 0;
    for (; src[i]; ++i) dst[i] = src[i];
    dst[i] = 0;   // закрыть строку
}

// =====================================================================
// РИСОВАНИЕ ТАБЛИЦЫ
// =====================================================================

// Нарисовать горизонтальную линию таблицы из символов '-'.
void printLine(int wPower, int wArray, int wList, int wBool, int wWord) {
    cout << '+' << string(wPower + 2, '-')
         << '+' << string(wArray + 2, '-')
         << '+' << string(wList  + 2, '-')
         << '+' << string(wBool  + 2, '-')
         << '+' << string(wWord  + 2, '-')
         << '+' << '\n';
    // string(n, c) — строка из n символов c. В C аналог — memset/malloc.
}

// Нарисовать одну строку таблицы.
void printRow(const string& p, const string& a, const string& l,
              const string& b, const string& w,
              int wPower, int wArray, int wList, int wBool, int wWord) {
    // const string& — ссылка на строку без копирования, только для чтения.
    cout << "| " << left << setw(wPower) << p
         << " | " << setw(wArray)  << a
         << " | " << setw(wList)   << l
         << " | " << setw(wBool)   << b
         << " | " << setw(wWord)   << w
         << " |" << '\n';
    // setw(n) — ширина поля вывода (аналог "%*s" в printf).
    // left — выравнивание по левому краю.
}

// =====================================================================
// ГЛАВНАЯ ФУНКЦИЯ
// =====================================================================
int main() {
    // Настройка кодировки консоли Windows. В C++ main всегда int.
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    srand(static_cast<unsigned int>(time(0)));  // инициализация rand()

    // Четыре исходных множества A, B, C, D.
    unsigned char A[LENOFLETTERS + 1] = { 0 }, B[LENOFLETTERS + 1] = { 0 };
    unsigned char C[LENOFLETTERS + 1] = { 0 }, D[LENOFLETTERS + 1] = { 0 };
    // Булевы версии тех же множеств.
    bool AB[LENOFLETTERS], BB[LENOFLETTERS], CB[LENOFLETTERS], DB[LENOFLETTERS];

    cout << "=== Comparative study of set representations ===\n";
    cout << "Formula: ((D and C) or A) and (not B)\n";
    cout << "Universe: a..z (26 letters)\n\n";

    // reps — сколько раз генерируем НОВЫЕ A, B, C, D,
    //        чтобы усреднить время по разным входным данным.
    // INNER — сколько раз внутри одного замера прогоняем формулу,
    //         чтобы таймер успел накопить измеримое время
    //         (одна операция короче разрешения таймера).
    const int reps  = 100;
    const int INNER = 1000;

    cout << "=== Timing table ===\n";
    cout << "Each cell: average time of one formula execution, microseconds (us)\n";
    cout << "For each power: " << reps << " random generations, each measured over "
         << INNER << " inner runs.\n\n";

    // Ширины колонок таблицы.
    const int wPower = 7;
    const int wArray = 18;
    const int wList  = 18;
    const int wBool  = 18;
    const int wWord  = 18;

    // Рисуем шапку таблицы.
    printLine(wPower, wArray, wList, wBool, wWord);
    printRow("Power", "Array (chars)", "List", "Bool array", "Machine word",
             wPower, wArray, wList, wBool, wWord);
    printLine(wPower, wArray, wList, wBool, wWord);

    // Основной цикл по мощностям множеств: 2, 4, 6, ..., 26.
    for (int len = 2; len <= 26; len += 2) {
        int realLen = (len > LENOFLETTERS) ? LENOFLETTERS : len;  // защита
        double sumA = 0, sumL = 0, sumB = 0, sumW = 0;

        // 100 раз генерируем новые множества и измеряем время.
        for (int it = 0; it < reps; ++it) {
            Discharge(A); Discharge(B); Discharge(C); Discharge(D);
            Generator(realLen, A); Generator(realLen, B);
            Generator(realLen, C); Generator(realLen, D);

            // ---------- Array ----------
            {
                // Копии исходных множеств: операции мутируют dst,
                // поэтому нельзя работать над A, B, C, D напрямую.
                unsigned char a1[LENOFLETTERS + 1], b1[LENOFLETTERS + 1];
                unsigned char c1[LENOFLETTERS + 1], d1[LENOFLETTERS + 1];

                // Замеряем INNER прогонов подряд.
                auto t1 = high_resolution_clock::now();
                for (int k = 0; k < INNER; ++k) { //1000 прогонов подряд
                    CopySet(A, a1); CopySet(B, b1);
                    CopySet(C, c1); CopySet(D, d1);
                    AndSets(d1, c1);        // C = D ∩ C
                    UnionSets(c1, a1);      // A = A ∪ C
                    ExclusionSets(b1, a1);  // A = A \ B
                }
                auto t2 = high_resolution_clock::now();

                // Переводим наносекунды в микросекунды и делим на INNER.
                sumA += duration_cast<nanoseconds>(t2 - t1).count() / 1000.0 / INNER;
            }

            // ---------- List ----------
            {
                auto t1 = high_resolution_clock::now();
                for (int k = 0; k < INNER; ++k) {
                    // Списки создаются заново каждый прогон, потому что
                    // операции их мутируют, а удаление — деструктивное.
                    ST* AL = ToList(A); ST* BL = ToList(B);
                    ST* CL = ToList(C); ST* DL = ToList(D);
                    AndSets(DL, CL); UnionSets(CL, AL); ExclusionSets(BL, AL);
                    delete AL; delete BL; delete CL; delete DL;
                }
                auto t2 = high_resolution_clock::now();
                sumL += duration_cast<nanoseconds>(t2 - t1).count() / 1000.0 / INNER;
            }

            // ---------- Bool ----------
            {
                auto t1 = high_resolution_clock::now();
                for (int k = 0; k < INNER; ++k) {
                    // Каждый раз очищаем и заново заполняем — иначе
                    // |= будет накапливать единицы.
                    ClearBool(AB); ClearBool(BB); ClearBool(CB); ClearBool(DB);
                    ToBool(A, AB); ToBool(B, BB); ToBool(C, CB); ToBool(D, DB);
                    AndSets(DB, CB); UnionSets(CB, AB); ExclusionSets(BB, AB);
                }
                auto t2 = high_resolution_clock::now();
                sumB += duration_cast<nanoseconds>(t2 - t1).count() / 1000.0 / INNER;
            }

            // ---------- Word ----------
            {
                // Готовим исходные маски один раз.
                int AW = ToMachineWord(A), BW = ToMachineWord(B);
                int CW = ToMachineWord(C), DW = ToMachineWord(D);

                auto t1 = high_resolution_clock::now();
                for (int k = 0; k < INNER; ++k) {
                    // Копируем в локальные переменные, потому что
                    // AndSets/UnionSets/ExclusionSets меняют свой 2-й аргумент.
                    int aw = AW, bw = BW, cw = CW, dw = DW;
                    AndSets(dw, cw); UnionSets(cw, aw); ExclusionSets(bw, aw);
                }
                auto t2 = high_resolution_clock::now();
                sumW += duration_cast<nanoseconds>(t2 - t1).count() / 1000.0 / INNER;
            }
        }

        // Печать строки таблицы для данной мощности.
        // snprintf — из C, форматирует число в строку. В C++ это делает
        // to_string, но snprintf даёт точный контроль над форматом.
        char bufP[16]; snprintf(bufP, sizeof(bufP), "%d", realLen);
        char bufA[32], bufL[32], bufB[32], bufW[32];
        snprintf(bufA, sizeof(bufA), "%.6f", sumA / reps);
        snprintf(bufL, sizeof(bufL), "%.6f", sumL / reps);
        snprintf(bufB, sizeof(bufB), "%.6f", sumB / reps);
        snprintf(bufW, sizeof(bufW), "%.6f", sumW / reps);

        printRow(bufP, bufA, bufL, bufB, bufW,
                 wPower, wArray, wList, wBool, wWord);
    }

    printLine(wPower, wArray, wList, wBool, wWord);

    // =================================================================
    // ДЕМОНСТРАЦИОННЫЙ ПРОГОН: показываем, что все 4 способа дают
    // ОДИН И ТОТ ЖЕ результат на одних и тех же входных множествах.
    // =================================================================
    cout << "\n=== Single demonstration run ===\n";
    Discharge(A); Discharge(B); Discharge(C); Discharge(D);
    Generator(10, A); Generator(10, B); Generator(10, C); Generator(10, D);

    cout << "Input data:\n";
    cout << "  A = " << A << '\n';
    cout << "  B = " << B << '\n';
    cout << "  C = " << C << '\n';
    cout << "  D = " << D << '\n';

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

    // Word
    int AW = ToMachineWord(A), BW = ToMachineWord(B);
    int CW = ToMachineWord(C), DW = ToMachineWord(D);
    AndSets(DW, CW); UnionSets(CW, AW); ExclusionSets(BW, AW);
    cout << "Machine word result:  "; PrintSet(AW); cout << '\n';

    return 0;
}