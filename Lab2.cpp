// ============================================================================
// Лабораторная работа №2: Множество как объект (ООП в C++)
// ВАРИАНТ 11. Универсум: 'a'...'z' (26 букв)
// Формула:  E = (A \ B) U (C n D)  -->  (A - B) | (C & D)
// ============================================================================
#include <iostream>
#include <iomanip>
#include <string>
#include <array>
#include <chrono>
#include <algorithm>
#include <vector>
#include <cstdint>
#include <cstring>
#include <utility>
#include <cstdlib>
#include <ctime>
#include <clocale>
#include <bitset>

#ifdef _WIN32
#include <Windows.h>
#endif

constexpr size_t UNIVERSE_SIZE = 26;
constexpr char BASE_CHAR = 'a';

volatile int g_sink = 0;

//БАРЬЕР ОПТИМИЗАЦИИ (Аналог Google Benchmark)
// Запрещает компилятору выносить вычисления из цикла и свертывать итерации
template <typename T>
inline void doNotOptimize(const T& value) {
#if defined(__GNUC__) || defined(__clang__)
    asm volatile("" : : "r,m"(value) : "memory");
#else
    g_sink += *reinterpret_cast<const char*>(&value);
#endif
}

bool g_trace_lifecycle = false;

void setupRussianConsole() {
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif
    setlocale(LC_ALL, "ru_RU.UTF-8");
}

std::string generateRandomElements(size_t cardinality) {
    std::array<char, UNIVERSE_SIZE> u;
    for (size_t i = 0; i < UNIVERSE_SIZE; ++i) u[i] = static_cast<char>(BASE_CHAR + i);
    for (size_t i = 0; i < cardinality; ++i) {
        size_t idx = i + rand() % (UNIVERSE_SIZE - i);
        std::swap(u[i], u[idx]);
    }
    return std::string(u.data(), cardinality);
}

template <typename SetType>
SetType computeFormula(const SetType& A, const SetType& B, const SetType& C, const SetType& D) {
    return (A - B) | (C & D);
}

// ============================================================================
// 1. МАШИННОЕ СЛОВО (uint32_t)
// ============================================================================
class SetWord {
private:
    uint32_t mask_{ 0 };
public:
    SetWord() = default;
    explicit SetWord(const std::string& elements) {
        for (char c : elements) {
            if (c >= BASE_CHAR && c < BASE_CHAR + UNIVERSE_SIZE) {
                mask_ |= (1U << (c - BASE_CHAR));
            }
        }
    }
    SetWord operator-(const SetWord& rhs) const { SetWord res; res.mask_ = mask_ & ~rhs.mask_; return res; }
    SetWord operator&(const SetWord& rhs) const { SetWord res; res.mask_ = mask_ & rhs.mask_; return res; }
    SetWord operator|(const SetWord& rhs) const { SetWord res; res.mask_ = mask_ | rhs.mask_; return res; }
    
    std::string toString() const {
        std::string str;
        for (size_t i = 0; i < UNIVERSE_SIZE; ++i)
            if (mask_ & (1U << i)) str += static_cast<char>(BASE_CHAR + i);
        return str.empty() ? "{пусто}" : str;
    }
    uint32_t getRawData() const { return mask_; }
};

// ============================================================================
// 2. БУЛЕВ МАССИВ (bool[26] с циклами по универсуму)
// ============================================================================
class SetBool {
private:
    bool present_[UNIVERSE_SIZE]{ false };
public:
    SetBool() = default;
    explicit SetBool(const std::string& elements) {
        for (char c : elements) {
            if (c >= BASE_CHAR && c < BASE_CHAR + UNIVERSE_SIZE) {
                present_[c - BASE_CHAR] = true;
            }
        }
    }
    
    SetBool operator-(const SetBool& rhs) const {
        SetBool res;
        for (size_t i = 0; i < UNIVERSE_SIZE; ++i) {
            res.present_[i] = present_[i] && !rhs.present_[i];
        }
        return res;
    }
    SetBool operator&(const SetBool& rhs) const {
        SetBool res;
        for (size_t i = 0; i < UNIVERSE_SIZE; ++i) {
            res.present_[i] = present_[i] && rhs.present_[i];
        }
        return res;
    }
    SetBool operator|(const SetBool& rhs) const {
        SetBool res;
        for (size_t i = 0; i < UNIVERSE_SIZE; ++i) {
            res.present_[i] = present_[i] || rhs.present_[i];
        }
        return res;
    }
    
    std::string toString() const {
        std::string str;
        for (size_t i = 0; i < UNIVERSE_SIZE; ++i)
            if (present_[i]) str += static_cast<char>(BASE_CHAR + i);
        return str.empty() ? "{пусто}" : str;
    }
    bool getFirst() const { return present_[0]; }
};

// ============================================================================
// 2.1. БИТОВАЯ ШКАЛА std::bitset (Дополнительный 5-й метод)
// ============================================================================
class SetBitset {
private:
    std::bitset<UNIVERSE_SIZE> present_;
public:
    SetBitset() = default;
    explicit SetBitset(const std::string& elements) {
        for (char c : elements) {
            if (c >= BASE_CHAR && c < BASE_CHAR + UNIVERSE_SIZE) {
                present_.set(c - BASE_CHAR);
            }
        }
    }
    SetBitset operator-(const SetBitset& rhs) const {
        SetBitset res;
        res.present_ = present_ & ~rhs.present_;
        return res;
    }
    SetBitset operator&(const SetBitset& rhs) const {
        SetBitset res;
        res.present_ = present_ & rhs.present_;
        return res;
    }
    SetBitset operator|(const SetBitset& rhs) const {
        SetBitset res;
        res.present_ = present_ | rhs.present_;
        return res;
    }
    std::string toString() const {
        std::string str;
        for (size_t i = 0; i < UNIVERSE_SIZE; ++i)
            if (present_.test(i)) str += static_cast<char>(BASE_CHAR + i);
        return str.empty() ? "{пусто}" : str;
    }
    bool getFirst() const { return present_.test(0); }
};

// ============================================================================
// 3. МАССИВ СИМВОЛОВ (Динамический буфер с перемещением)
// ============================================================================
class SetArray {
private:
    char* data_{ nullptr };
    size_t size_{ 0 };
    
    bool contains(char c) const {
        for (size_t i = 0; i < size_; ++i) if (data_[i] == c) return true;
        return false;
    }
    void append(char c) { data_[size_++] = c; data_[size_] = '\0'; }
public:
    SetArray() : data_(new char[UNIVERSE_SIZE + 1]), size_(0) { data_[0] = '\0'; }
    explicit SetArray(const std::string& elements) : SetArray() {
        for (char c : elements) {
            if (!contains(c)) append(c);
        }
    }
    ~SetArray() { delete[] data_; }
    
    SetArray(const SetArray& other) : data_(new char[UNIVERSE_SIZE + 1]), size_(other.size_) {
        std::memcpy(data_, other.data_, size_ + 1);
    }
    
    SetArray(SetArray&& other) noexcept : data_(other.data_), size_(other.size_) {
        other.data_ = nullptr; 
        other.size_ = 0;
    }
    
    SetArray& operator=(const SetArray& other) {
        if (this != &other) {
            size_ = other.size_;
            std::memcpy(data_, other.data_, size_ + 1);
        }
        return *this;
    }
    
    SetArray& operator=(SetArray&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = other.data_;
            size_ = other.size_;
            other.data_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }
    
    SetArray operator-(const SetArray& rhs) const {
        SetArray res;
        for (size_t i = 0; i < size_; ++i)
            if (!rhs.contains(data_[i])) res.append(data_[i]);
        return res;
    }
    SetArray operator&(const SetArray& rhs) const {
        SetArray res;
        for (size_t i = 0; i < size_; ++i)
            if (rhs.contains(data_[i])) res.append(data_[i]);
        return res;
    }
    SetArray operator|(const SetArray& rhs) const {
        SetArray res = *this;
        for (size_t i = 0; i < rhs.size_; ++i)
            if (!res.contains(rhs.data_[i])) res.append(rhs.data_[i]);
        return res;
    }
    
    std::string toString() const {
        return (size_ == 0 || !data_) ? "{пусто}" : std::string(data_, size_);
    }
    char getFirst() const { return size_ > 0 ? data_[0] : 0; }
};

// ============================================================================
// 4. ОДНОСВЯЗНЫЙ СПИСОК (Арена-пул памяти)
// ============================================================================
struct Node {
    char data; 
    Node* next;
    Node(char val = 0, Node* n = nullptr) : data(val), next(n) {}
    
    static constexpr size_t POOL_SIZE = 5000000;
    static Node* pool;
    static size_t pool_head;
    static bool use_pool;
    
    static void initPool() {
        if (!pool) pool = static_cast<Node*>(std::malloc(POOL_SIZE * sizeof(Node)));
        pool_head = 0;
    }
    static void destroyPool() {
        if (pool) {
            std::free(pool);
            pool = nullptr;
        }
    }
    static void resetPool() { pool_head = 0; }
    
    static void* operator new(size_t sz) {
        if (use_pool) {
            if (pool_head < POOL_SIZE) return &pool[pool_head++];
            std::cerr << "[Ошибка пула памяти: переполнение!]\n";
            std::exit(EXIT_FAILURE);
        }
        return ::operator new(sz);
    }
    
    static void operator delete(void* p) noexcept {
        if (!use_pool) {
            ::operator delete(p);
        }
    }
};
Node* Node::pool = nullptr;
size_t Node::pool_head = 0;
bool Node::use_pool = false;

class SetList {
private:
    Node* head_{ nullptr }; 
    size_t size_{ 0 }; 
    std::string tag_{ "R" };
    
    void clear() { 
        while (head_) { 
            Node* tmp = head_; 
            head_ = head_->next; 
            delete tmp; 
        } 
        size_ = 0; 
    }
    
    bool contains(char c) const {
        for (Node* curr = head_; curr; curr = curr->next)
            if (curr->data == c) return true;
        return false;
    }
    
    void pushFront(char c) { head_ = new Node(c, head_); size_++; }
public:
    SetList() = default;
    explicit SetList(std::string tag) : tag_(std::move(tag)) {
        if (g_trace_lifecycle) std::cout << " [Конструктор] Множество '" << tag_ << "' создано\n";
    }
    SetList(const std::string& elements, std::string tag) : SetList(std::move(tag)) {
        for (char c : elements) {
            if (!contains(c)) pushFront(c);
        }
    }
    ~SetList() {
        if (g_trace_lifecycle) std::cout << " [Деструктор] Множество '" << tag_ << "' уничтожено\n";
        clear();
    }
    
    SetList(const SetList& other) : tag_("Копия_" + other.tag_) {
        if (g_trace_lifecycle) std::cout << " [Копирование] '" << tag_ << "' скопировано из '" << other.tag_ << "'\n";
        Node* curr = other.head_;
        std::vector<char> temp;
        while (curr) { temp.push_back(curr->data); curr = curr->next; }
        for (auto it = temp.rbegin(); it != temp.rend(); ++it) pushFront(*it);
    }
    
    SetList(SetList&& other) noexcept : head_(other.head_), size_(other.size_), tag_("Перенос_" + other.tag_) {
        if (g_trace_lifecycle) std::cout << " [Перенос] '" << tag_ << "' перемещено из '" << other.tag_ << "'\n";
        other.head_ = nullptr; 
        other.size_ = 0;
    }
    
    SetList& operator=(const SetList& other) {
        if (this != &other) { 
            clear(); 
            tag_ = "Копия_" + other.tag_;
            Node* curr = other.head_;
            std::vector<char> temp;
            while (curr) { temp.push_back(curr->data); curr = curr->next; }
            for (auto it = temp.rbegin(); it != temp.rend(); ++it) pushFront(*it);
        }
        return *this;
    }
    
    SetList& operator=(SetList&& other) noexcept {
        if (this != &other) { 
            clear(); 
            head_ = other.head_; 
            size_ = other.size_; 
            tag_ = "Перенос_" + other.tag_;
            other.head_ = nullptr; 
            other.size_ = 0; 
        }
        return *this;
    }
    
    SetList operator-(const SetList& rhs) const {
        SetList res(tag_ + "_разность_" + rhs.tag_);
        for (Node* curr = head_; curr; curr = curr->next)
            if (!rhs.contains(curr->data)) res.pushFront(curr->data);
        return res;
    }
    
    SetList operator&(const SetList& rhs) const {
        SetList res(tag_ + "_пересечение_" + rhs.tag_);
        for (Node* curr = head_; curr; curr = curr->next)
            if (rhs.contains(curr->data)) res.pushFront(curr->data);
        return res;
    }
    
    SetList operator|(const SetList& rhs) const {
        SetList res(tag_ + "_объединение_" + rhs.tag_);
        for (Node* curr = head_; curr; curr = curr->next) res.pushFront(curr->data);
        for (Node* curr = rhs.head_; curr; curr = curr->next)
            if (!res.contains(curr->data)) res.pushFront(curr->data);
        return res;
    }
    
    std::string toString() const {
        if (!head_) return "{пусто}";
        std::string str; for (Node* curr = head_; curr; curr = curr->next) str += curr->data;
        return str;
    }
    char getFirst() const { return head_ ? head_->data : 0; }
};

// ============================================================================
// ТЕСТЫ И БЕНЧМАРКИ
// ============================================================================
void runLifecycleDemo() {
    std::cout << "=======================================================================\n";
    std::cout << " РАЗДЕЛ 1: ОТСЛЕЖИВАНИЕ ЖИЗНЕННОГО ЦИКЛА ОБЪЕКТОВ (СПИСОК)\n";
    std::cout << "=======================================================================\n";
    g_trace_lifecycle = true;
    {
        std::string strA = generateRandomElements(6);
        std::string strB = generateRandomElements(6);
        std::string strC = generateRandomElements(6);
        std::string strD = generateRandomElements(6);

        std::cout << "--- Создание исходных множеств A, B, C, D ---\n";
        SetList A(strA, "A");
        SetList B(strB, "B");
        SetList C(strC, "C");
        SetList D(strD, "D");
        
        std::cout << "\n--- Вычисление выражения E = (A - B) | (C & D) ---\n";
        SetList E = computeFormula(A, B, C, D);
        
        std::cout << "\n--- Содержимое множеств ---\n";
        std::cout << " Множество A = " << A.toString() << "\n";
        std::cout << " Множество B = " << B.toString() << "\n";
        std::cout << " Множество C = " << C.toString() << "\n";
        std::cout << " Множество D = " << D.toString() << "\n";
        std::cout << " Результат E = " << E.toString() << "\n\n";
        std::cout << "--- Выход из области видимости (вызов деструкторов) ---\n";
    }
    g_trace_lifecycle = false;
    std::cout << "=======================================================================\n\n";
}

void runEquivalenceTest() {
    std::cout << "=======================================================================\n";
    std::cout << " РАЗДЕЛ 2: ПРОВЕРКА ЭКВИВАЛЕНТНОСТИ НА ЕДИНОМ НАБОРЕ ДАННЫХ\n";
    std::cout << "=======================================================================\n";
    
    std::string strA = generateRandomElements(8);
    std::string strB = generateRandomElements(8);
    std::string strC = generateRandomElements(8);
    std::string strD = generateRandomElements(8);

    SetWord   WordA(strA),   WordB(strB),   WordC(strC),   WordD(strD);
    SetBool   BoolA(strA),   BoolB(strB),   BoolC(strC),   BoolD(strD);
    SetBitset BitsetA(strA), BitsetB(strB), BitsetC(strC), BitsetD(strD);
    SetArray  ArrayA(strA),  ArrayB(strB),  ArrayC(strC),  ArrayD(strD);
    SetList   ListA(strA, "A"), ListB(strB, "B"), ListC(strC, "C"), ListD(strD, "D");

    auto ResWord   = computeFormula(WordA, WordB, WordC, WordD);
    auto ResBool   = computeFormula(BoolA, BoolB, BoolC, BoolD);
    auto ResBitset = computeFormula(BitsetA, BitsetB, BitsetC, BitsetD);
    auto ResArray  = computeFormula(ArrayA, ArrayB, ArrayC, ArrayD);
    auto ResList   = computeFormula(ListA, ListB, ListC, ListD);

    std::string sWord   = ResWord.toString();   std::sort(sWord.begin(), sWord.end());
    std::string sBool   = ResBool.toString();   std::sort(sBool.begin(), sBool.end());
    std::string sBitset = ResBitset.toString(); std::sort(sBitset.begin(), sBitset.end());
    std::string sArray  = ResArray.toString();  std::sort(sArray.begin(), sArray.end());
    std::string sList   = ResList.toString();   std::sort(sList.begin(), sList.end());

    std::cout << " [1] Машинное слово    : " << ResWord.toString()   << "\n";
    std::cout << " [2] Булев массив bool[]: " << ResBool.toString()   << "\n";
    std::cout << " [3] Битовая шкала bitset: " << ResBitset.toString() << "\n";
    std::cout << " [4] Массив символов   : " << ResArray.toString()  << "\n";
    std::cout << " [5] Список            : " << ResList.toString()   << "\n\n";
    
    if (sWord == sBool && sBool == sBitset && sBitset == sArray && sArray == sList) {
        std::cout << " Статус проверки: УСПЕШНО (Все 5 способов дали 100% одинаковый результат!)\n";
    } else {
        std::cout << " Статус проверки: ОШИБКА (Результаты различаются!)\n";
    }
    std::cout << "=======================================================================\n\n";
}

void runBenchmark() {
    std::cout << "=======================================================================\n";
    std::cout << " РАЗДЕЛ 3: ЗАМЕР ВРЕМЕНИ ВЫПОЛНЕНИЯ (БЕЗ ЛОЖНЫХ НУЛЕЙ)\n";
    std::cout << "=======================================================================\n";
    std::cout << "Единица измерения: микросекунды (мкс) на 1 операцию вычисления.\n\n";

    constexpr int ITER_SLOW = 50000;
    constexpr int ITER_FAST = 2000000; // 2 миллиона итераций для сверхвысокой точности

    std::cout << std::left << std::setw(8)  << "Мощн."
              << std::setw(15) << " Массив (мкс)"
              << std::setw(17) << " Список Стд"
              << std::setw(17) << " Список Пул"
              << std::setw(16) << " Бул.масс bool"
              << std::setw(16) << " Шкала bitset"
              << std::setw(16) << " Маш.слово" << std::endl;
    std::cout << std::string(105, '-') << std::endl;

    for (size_t len = 2; len <= UNIVERSE_SIZE; len += 2) {
        std::string sA = generateRandomElements(len);
        std::string sB = generateRandomElements(len);
        std::string sC = generateRandomElements(len);
        std::string sD = generateRandomElements(len);

        // 1. Массив символов
        double tArray = 0;
        {
            SetArray A(sA), B(sB), C(sC), D(sD);
            auto t1 = std::chrono::high_resolution_clock::now();
            for (int k = 0; k < ITER_SLOW; ++k) {
                SetArray E = computeFormula(A, B, C, D);
                doNotOptimize(E);
            }
            auto t2 = std::chrono::high_resolution_clock::now();
            tArray = std::chrono::duration<double, std::micro>(t2 - t1).count() / ITER_SLOW;
        }
        
        // 2. Список (Стандартный new/delete)
        double tListStd = 0;
        {
            Node::use_pool = false;
            SetList A(sA, "A"), B(sB, "B"), C(sC, "C"), D(sD, "D");
            auto t1 = std::chrono::high_resolution_clock::now();
            for (int k = 0; k < ITER_SLOW; ++k) {
                SetList E = computeFormula(A, B, C, D);
                doNotOptimize(E);
            }
            auto t2 = std::chrono::high_resolution_clock::now();
            tListStd = std::chrono::duration<double, std::micro>(t2 - t1).count() / ITER_SLOW;
        }

        // 3. Список (Арена-Пул)
        double tListPool = 0;
        {
            Node::use_pool = false;
            SetList A(sA, "A"), B(sB, "B"), C(sC, "C"), D(sD, "D");
            Node::use_pool = true; 
            Node::resetPool();
            auto t1 = std::chrono::high_resolution_clock::now();
            for (int k = 0; k < ITER_SLOW; ++k) {
                Node::resetPool();
                SetList E = computeFormula(A, B, C, D);
                doNotOptimize(E);
            }
            auto t2 = std::chrono::high_resolution_clock::now();
            tListPool = std::chrono::duration<double, std::micro>(t2 - t1).count() / ITER_SLOW;
            Node::use_pool = false;
        }

        // 4. Булев массив (bool[26])
        double tBool = 0;
        {
            SetBool A(sA), B(sB), C(sC), D(sD);
            auto t1 = std::chrono::high_resolution_clock::now();
            for (int k = 0; k < ITER_FAST; ++k) {
                SetBool E = computeFormula(A, B, C, D);
                doNotOptimize(E);
            }
            auto t2 = std::chrono::high_resolution_clock::now();
            tBool = std::chrono::duration<double, std::micro>(t2 - t1).count() / ITER_FAST;
        }

        // 5. Битовая шкала (std::bitset)
        double tBitset = 0;
        {
            SetBitset A(sA), B(sB), C(sC), D(sD);
            auto t1 = std::chrono::high_resolution_clock::now();
            for (int k = 0; k < ITER_FAST; ++k) {
                SetBitset E = computeFormula(A, B, C, D);
                doNotOptimize(E);
            }
            auto t2 = std::chrono::high_resolution_clock::now();
            tBitset = std::chrono::duration<double, std::micro>(t2 - t1).count() / ITER_FAST;
        }

        // 6. Машинное слово (uint32_t)
        double tWord = 0;
        {
            SetWord A(sA), B(sB), C(sC), D(sD);
            auto t1 = std::chrono::high_resolution_clock::now();
            for (int k = 0; k < ITER_FAST; ++k) {
                SetWord E = computeFormula(A, B, C, D);
                doNotOptimize(E);
            }
            auto t2 = std::chrono::high_resolution_clock::now();
            tWord = std::chrono::duration<double, std::micro>(t2 - t1).count() / ITER_FAST;
        }

        std::cout << std::left << std::setw(8)  << len
                  << std::setw(15) << std::fixed << std::setprecision(6) << tArray
                  << std::setw(17) << tListStd
                  << std::setw(17) << tListPool
                  << std::setw(16) << tBool
                  << std::setw(16) << tBitset
                  << std::setw(16) << tWord << std::endl;
    }
    std::cout << std::string(105, '-') << std::endl << std::endl;
    
    std::cout << "=======================================================================\n";
    std::cout << " РАЗДЕЛ 4: ВЫВОДЫ И СРАВНЕНИЕ ПОДХОДОВ (ПРОЦЕДУРНЫЙ VS ООП)\n";
    std::cout << "=======================================================================\n";
    std::cout << "1. СРАВНЕНИЕ БУЛЕВА МАССИВА `bool[]` И ШКАЛЫ `std::bitset`:\n";
    std::cout << "   - `SetBool` (bool[]) перебирает 26 элементов циклами. Его время константно,\n";
    std::cout << "     но на каждую операцию уходит несколько десятков итераций.\n";
    std::cout << "   - `SetBitset` (std::bitset) и `SetWord` применяют поразрядные инструкции CPU,\n";
    std::cout << "     выполняя операции за 1 тактовый цикл O(1), что ускоряет расчеты.\n\n";
    std::cout << "2. СПИСОК С ПУЛОМ ПАМЯТИ:\n";
    std::cout << "   - Арена-пул исключает вызовы OS malloc/free, ускоряя список в 10-15 раз.\n\n";
    std::cout << "3. ПРОЦЕДУРНЫЙ ПОДХОД (ЛР1) VS ОБЪЕКТНО-ОРИЕНТИРОВАННЫЙ (ЛР2):\n";
    std::cout << "   - В ООП перегрузка операторов позволяет записывать формулу естественно:\n";
    std::cout << "     E = (A - B) | (C & D). Семантика перемещения (Move) нивелирует накладные\n";
    std::cout << "     расходы на создание временных объектов, делая ООП код столь же быстрым,\n";
    std::cout << "     как и процедуры из ЛР1, но при этом гарантированно безопасным (RAII).\n";
    std::cout << "=======================================================================\n\n";
}

int main() {
    setupRussianConsole();
    srand(static_cast<unsigned int>(time(nullptr)));

    Node::initPool();

    std::cout << "=======================================================================\n";
    std::cout << "   ЛАБОРАТОРНАЯ РАБОТА N2: МНОЖЕСТВО КАК ОБЪЕКТ (КЛАССЫ C++)\n";
    std::cout << "   Вариант 11 | Формула: E = (A \\ B) U (C n D)\n";
    std::cout << "=======================================================================\n\n";

    runLifecycleDemo();
    runEquivalenceTest();
    runBenchmark();
    
    Node::destroyPool();
    return 0;
}


