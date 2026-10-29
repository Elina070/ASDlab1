// ============================================================================
// Лабораторная работа №3: ДЕРЕВЬЯ
// ВАРИАНТ 11
// ----------------------------------------------------------------------------
//  - Вид дерева: Двоичное (Binary Tree)
//  - Разметка вершин: Прямая (Pre-order: Корень -> Левый -> Правый)
//  - Способ обхода для вывода: Внутренний (In-order: Левый -> Корень -> Правый)
//  - Расчет: Уровень <= 2 (вариант 11), Уровень > 2, Всего вершин
// ============================================================================

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>
#include <random>
#include <cstdint>
#include <clocale>

#ifdef _WIN32
#include <Windows.h>
#endif

// Глобальный барьер оптимизации
volatile int g_sink = 0;
template <typename T>
inline void doNotOptimize(const T& value) {
#if defined(__GNUC__) || defined(__clang__)
    asm volatile("" : : "r,m"(value) : "memory");
#else
    // В MSVC используем результат, чтобы он не считался полностью ненужным.
    g_sink = g_sink + static_cast<int>(value);
#endif
}

void setupRussianConsole() {
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif
    std::setlocale(LC_ALL, "ru_RU.UTF-8");
}

// Подсчет установленных бит; конкретная реализация зависит от компилятора и платформы
inline int countBits(uint64_t v) {
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_popcountll(v);
#else
    int count = 0;
    while (v) {
        v &= (v - 1);
        count++;
    }
    return count;
#endif
}

struct Node;

// ============================================================================
// СПОСОБ 1: КЛАССИЧЕСКОЕ ДЕРЕВО НА УКАЗАТЕЛЯХ (PointerTree)
// ============================================================================

struct Node {
    char data;          
    int label{ 0 };     
    Node* left{ nullptr };
    Node* right{ nullptr };

    Node(char val) : data(val) {}

    ~Node() {
        delete left;
        delete right;
    }
};

class PointerTree {
private:
    Node* root_{ nullptr };

    void labelPreOrder(Node* node, int& current_label) {
        if (!node) return;
        node->label = ++current_label;
        labelPreOrder(node->left, current_label);
        labelPreOrder(node->right, current_label);
    }

    void printInOrder(Node* node) const {
        if (!node) return;
        printInOrder(node->left);
        std::cout << "[" << node->label << ":" << node->data << "] ";
        printInOrder(node->right);
    }

    Node* inputKeyboardRecursive() {
        char val;
        std::cout << "Введите символ узла (или '.' если ветки нет): ";
        std::cin >> val;

        if (val == '.') return nullptr;

        Node* node = new Node(val);

        std::cout << " -> Идем в ЛЕВОГО ребенка для '" << val << "'\n";
        node->left = inputKeyboardRecursive();

        std::cout << " -> Идем в ПРАВОГО ребенка для '" << val << "'\n";
        node->right = inputKeyboardRecursive();

        return node;
    }

    void printPrettyTree(Node* node, const std::string& prefix = "", bool isTail = true, bool isRoot = true) const {
        if (!node) return;

        if (isRoot) {
            std::cout << "M" << node->label << "(" << node->data << ") [КОРЕНЬ]\n";
        } else {
            std::cout << prefix << (isTail ? "└── " : "├── ") 
                      << "M" << node->label << "(" << node->data << ")\n";
        }

        std::vector<Node*> children;
        if (node->left)  children.push_back(node->left);
        if (node->right) children.push_back(node->right);

        for (size_t i = 0; i < children.size(); ++i) {
            bool tail = (i == children.size() - 1);
            std::string newPrefix = isRoot ? "" : (prefix + (isTail ? "    " : "│   "));
            printPrettyTree(children[i], newPrefix, tail, false);
        }
    }

    int countLevelSub2(Node* node, int current_depth) const {
        if (!node || current_depth > 2) return 0;
        return 1 + countLevelSub2(node->left, current_depth + 1)
                 + countLevelSub2(node->right, current_depth + 1);
    }

    int countLevelAbove2(Node* node, int current_depth) const {
        if (!node) return 0;
        int self = (current_depth > 2) ? 1 : 0;
        return self + countLevelAbove2(node->left, current_depth + 1)
                    + countLevelAbove2(node->right, current_depth + 1);
    }

    int countTotal(Node* node) const {
        if (!node) return 0;
        return 1 + countTotal(node->left) + countTotal(node->right);
    }

public:
    PointerTree() = default;
    ~PointerTree() { delete root_; }

    PointerTree(const PointerTree&) = delete;
    PointerTree& operator=(const PointerTree&) = delete;

    Node* getRoot() const { return root_; }

    void buildFromKeyboard() {
        delete root_;
        root_ = nullptr;
        std::cout << "\n=== РЕЖИМ ВВОДА ДЕРЕВА С КЛАВИАТУРЫ ===\n";
        std::cout << "Инструкция: вводите символы узлов. Введите '.' чтобы указать отсутствие ветки.\n\n";
        root_ = inputKeyboardRecursive();

        int label_counter = 0;
        labelPreOrder(root_, label_counter);
    }

    void generateRandom(int max_nodes) {
        delete root_;
        root_ = nullptr;

        if (max_nodes <= 0) return;

        std::mt19937 rng(1337);
        root_ = new Node('a');
        int created = 1;

        // Для N <= 63 выбираем случайные свободные позиции в heap-представлении.
        // Благодаря этому BitmaskTree сможет представить сгенерированное дерево.
        if (max_nodes <= 63) {
            std::vector<Node*> slots(63, nullptr);
            slots[0] = root_;

            while (created < max_nodes) {
                std::vector<int> available;
                for (int i = 1; i < 63; ++i) {
                    const int parentIndex = (i - 1) / 2;
                    if (slots[i] == nullptr && slots[parentIndex] != nullptr) {
                        available.push_back(i);
                    }
                }
                if (available.empty()) break;

                const int index = available[rng() % available.size()];
                Node* node = new Node(static_cast<char>('a' + (created % 26)));
                slots[index] = node;
                Node* parent = slots[(index - 1) / 2];
                if (index % 2 == 1) parent->left = node;
                else parent->right = node;
                ++created;
            }
        } else {
            // Для больших деревьев продолжаем генерацию, но битовая версия
            // может оказаться неприменима из-за ограничения одной uint64_t.
            std::vector<Node*> current_layer = { root_ };
            while (created < max_nodes && !current_layer.empty()) {
                std::vector<Node*> next_layer;
                for (Node* parent : current_layer) {
                    if (created < max_nodes && !parent->left) {
                        parent->left = new Node(static_cast<char>('a' + (created % 26)));
                        next_layer.push_back(parent->left);
                        ++created;
                    }
                    if (created < max_nodes && !parent->right) {
                        parent->right = new Node(static_cast<char>('a' + (created % 26)));
                        next_layer.push_back(parent->right);
                        ++created;
                    }
                }
                current_layer = std::move(next_layer);
            }
        }

        int label_counter = 0;
        labelPreOrder(root_, label_counter);
    }

    void displayStructure() const {
        std::cout << "\n--- ИЗОБРАЖЕНИЕ СТРУКТУРЫ ДЕРЕВА ---\n";
        if (!root_) {
            std::cout << "Дерево пустое!\n";
        } else {
            printPrettyTree(root_);
        }
        std::cout << "------------------------------------\n";
    }

    void displayInOrder() const {
        std::cout << "Последовательность меток при ВНУТРЕННЕМ обходе (In-Order):\n";
        printInOrder(root_);
        std::cout << "\n";
    }

    int getNodesCountLevelSub2() const { return countLevelSub2(root_, 0); }
    int getNodesCountLevelAbove2() const { return countLevelAbove2(root_, 0); }
    int getTotalNodesCount() const { return countTotal(root_); }
};

// ============================================================================
// СПОСОБ 2: ДЕРЕВО В ВЕКТОРЕ (FlatTree)
// ============================================================================

struct FlatNode {
    char data;
    int label{ 0 };
    int left{ -1 };
    int right{ -1 };
};

class FlatTree {
private:
    std::vector<FlatNode> nodes_;

    int countLevelSub2(int idx, int depth) const {
        if (idx == -1 || depth > 2) return 0;
        return 1 + countLevelSub2(nodes_[idx].left, depth + 1)
                 + countLevelSub2(nodes_[idx].right, depth + 1);
    }

    int countLevelAbove2(int idx, int depth) const {
        if (idx == -1) return 0;
        int self = (depth > 2) ? 1 : 0;
        return self + countLevelAbove2(nodes_[idx].left, depth + 1)
                    + countLevelAbove2(nodes_[idx].right, depth + 1);
    }

    int convertFromNode(const Node* node) {
        if (!node) return -1;

        int current_idx = static_cast<int>(nodes_.size());
        nodes_.push_back({ node->data, node->label, -1, -1 });

        int left_child = convertFromNode(node->left);
        int right_child = convertFromNode(node->right);

        nodes_[current_idx].left = left_child;
        nodes_[current_idx].right = right_child;

        return current_idx;
    }

public:
    void buildFromPointerTree(const PointerTree& src) {
        nodes_.clear();
        if (src.getRoot()) {
            convertFromNode(src.getRoot());
        }
    }

    int getNodesCountLevelSub2() const {
        if (nodes_.empty()) return 0;
        return countLevelSub2(0, 0);
    }

    int getNodesCountLevelAbove2() const {
        if (nodes_.empty()) return 0;
        return countLevelAbove2(0, 0);
    }

    int getTotalNodesCount() const {
        return static_cast<int>(nodes_.size());
    }
};

// ============================================================================
// СПОСОБ 3: БИТОВОЕ ДЕРЕВО (BitmaskTree)
// ============================================================================

class BitmaskTree {
private:
    // Биты 0..62 используются для индексов узлов в представлении кучи.
    // Если хотя бы один узел не помещается, дерево считается неподдерживаемым.
    uint64_t mask_{ 0 };
    bool valid_{ true };

    void convertFromNode(const Node* node, int bit_index) {
        if (!node || !valid_) return;
        if (bit_index >= 63) {
            valid_ = false;
            return;
        }

        mask_ |= (uint64_t{1} << bit_index);
        convertFromNode(node->left, 2 * bit_index + 1);
        convertFromNode(node->right, 2 * bit_index + 2);
    }

public:
    void buildFromPointerTree(const PointerTree& src) {
        mask_ = 0;
        valid_ = true;
        if (src.getRoot()) convertFromNode(src.getRoot(), 0);
    }

    bool isRepresentable() const { return valid_; }

    int getNodesCountLevelSub2() const {
        if (!valid_) return -1;
        constexpr uint64_t LEVEL_0_1_2_MASK = 0x7F;
        return countBits(mask_ & LEVEL_0_1_2_MASK);
    }

    int getNodesCountLevelAbove2() const {
        if (!valid_) return -1;
        constexpr uint64_t LEVEL_0_1_2_MASK = 0x7F;
        return countBits(mask_ & ~LEVEL_0_1_2_MASK);
    }

    int getTotalNodesCount() const {
        if (!valid_) return -1;
        return countBits(mask_);
    }
};

// ============================================================================
// ГЛАВНАЯ ПРОГРАММА И БЕНЧМАРК
// ============================================================================

int main() {
    setupRussianConsole();

    std::cout << "=======================================================================\n";
    std::cout << "   ЛАБОРАТОРНАЯ РАБОТА N3: ДЕРЕВЬЯ (ООП И СТРУКТУРЫ ДАННЫХ)\n";
    std::cout << "   Вариант 11 | Двоичное дерево | Прямая разметка | Внутренний обход\n";
    std::cout << "=======================================================================\n\n";

    int mode = 0;
    std::cout << "Выберите способ построения дерева:\n";
    std::cout << "  [1] Ввод дерева с клавиатуры вручную\n";
    std::cout << "  [0] Автоматическая случайная генерация\n";
    std::cout << "Ваш выбор: ";
    std::cin >> mode;

    PointerTree pTree;

    if (mode == 1) {
        pTree.buildFromKeyboard();
    } else {
        int user_n = 15;
        std::cout << "\nВведите количество узлов N для генерации (например, 15): ";
        std::cin >> user_n;
        pTree.generateRandom(user_n);
    }

    pTree.displayStructure();
    pTree.displayInOrder();

    FlatTree    fTree;
    BitmaskTree bTree;

    fTree.buildFromPointerTree(pTree); 
    bTree.buildFromPointerTree(pTree); 

    std::cout << "\n=======================================================================\n";
    std::cout << " РАЗДЕЛ 2: ПОЛНАЯ СТАТИСТИКА УЗЛОВ (ДЛЯ 3-Х СТРУКТУР)\n";
    std::cout << "=======================================================================\n";
    std::cout << std::left << std::setw(28) << " Метрика / Структура"
              << std::setw(18) << "PointerTree"
              << std::setw(18) << "FlatTree"
              << std::setw(18) << "BitmaskTree" << "\n";
    std::cout << "-----------------------------------------------------------------------\n";

    std::cout << std::left << std::setw(28) << " [1] Уровень <= 2 (Вар 11):"
              << std::setw(18) << pTree.getNodesCountLevelSub2()
              << std::setw(18) << fTree.getNodesCountLevelSub2()
              << std::setw(18) << bTree.getNodesCountLevelSub2() << "\n";

    std::cout << std::left << std::setw(28) << " [2] Уровень > 2 (Ниже 2):"
              << std::setw(18) << pTree.getNodesCountLevelAbove2()
              << std::setw(18) << fTree.getNodesCountLevelAbove2()
              << std::setw(18) << bTree.getNodesCountLevelAbove2() << "\n";

    std::cout << std::left << std::setw(28) << " [3] Всего узлов в дереве:"
              << std::setw(18) << pTree.getTotalNodesCount()
              << std::setw(18) << fTree.getTotalNodesCount()
              << std::setw(18) << bTree.getTotalNodesCount() << "\n";
    std::cout << "-----------------------------------------------------------------------\n";

    const bool sameSub2 = pTree.getNodesCountLevelSub2() == fTree.getNodesCountLevelSub2()
        && fTree.getNodesCountLevelSub2() == bTree.getNodesCountLevelSub2();
    const bool sameAbove2 = pTree.getNodesCountLevelAbove2() == fTree.getNodesCountLevelAbove2()
        && fTree.getNodesCountLevelAbove2() == bTree.getNodesCountLevelAbove2();
    const bool sameTotal = pTree.getTotalNodesCount() == fTree.getTotalNodesCount()
        && fTree.getTotalNodesCount() == bTree.getTotalNodesCount();

    if (bTree.isRepresentable() && sameSub2 && sameAbove2 && sameTotal) {
        std::cout << " РЕЗУЛЬТАТ: УСПЕШНО — результаты всех трех структур совпадают.\n";
    } else if (!bTree.isRepresentable()) {
        std::cout << " РЕЗУЛЬТАТ: PointerTree и FlatTree построены корректно, но BitmaskTree\n";
        std::cout << " не может представить это дерево в одной 64-битной маске.\n";
    } else {
        std::cout << " РЕЗУЛЬТАТ: ОШИБКА — результаты структур расходятся.\n";
    }

    std::cout << "\n=======================================================================\n";
    std::cout << " РАЗДЕЛ 3: ЗАМЕР ПОДСЧЕТА ВСЕХ УЗЛОВ (1 000 000 ИТЕРАЦИЙ)\n";
    std::cout << "=======================================================================\n";

    constexpr int ITERATIONS = 1000000;
    using Clock = std::chrono::steady_clock;

    auto measureTotalCount = [=](auto&& tree) {
        auto start = Clock::now();
        for (int i = 0; i < ITERATIONS; ++i) {
            int count = tree.getTotalNodesCount();
            doNotOptimize(count);
        }
        auto finish = Clock::now();
        return std::chrono::duration<double, std::micro>(finish - start).count();
    };

    const double time_p = measureTotalCount(pTree);
    const double time_f = measureTotalCount(fTree);

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Операция: getTotalNodesCount(), всего " << ITERATIONS << " вызовов.\n";
    std::cout << std::left << std::setw(35) << " 1. Указатели (PointerTree): "
              << time_p << " мкс (обход дерева)\n";
    std::cout << std::left << std::setw(35) << " 2. Вектор    (FlatTree): "
              << time_f << " мкс (размер вектора)\n";

    if (bTree.isRepresentable()) {
        const double time_b = measureTotalCount(bTree);
        std::cout << std::left << std::setw(35) << " 3. Битовое   (BitmaskTree): "
                  << time_b << " мкс (подсчет битов)\n";
    } else {
        std::cout << " 3. Битовое   (BitmaskTree): замер пропущен — дерево не помещается в маску.\n";
    }

    std::cout << "\nВажно для анализа результатов:\n";
    std::cout << "- PointerTree каждый раз обходит узлы: O(n).\n";
    std::cout << "- FlatTree возвращает заранее известный размер вектора: O(1).\n";
    std::cout << "- BitmaskTree считает биты в одном uint64_t: O(1) для фиксированной маски.\n";
    std::cout << "Это сравнение времени конкретных реализаций операции, а не одинаковых алгоритмов.\n";
    std::cout << "=======================================================================\n";

    return 0;
}