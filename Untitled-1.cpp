#include <iostream>
#include <vector>
#include <clocale>
#include <Windows.h>

using namespace std;

// Настройка русского языка в консоли Windows
void setupRussianConsole() {
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    setlocale(LC_ALL, ".UTF-8");
}

class BinaryTree {
private:
    vector<bool> tree;

    // Ввод дерева с клавиатуры
    // 1 - создать вершину, 0 - пустое поддерево
    void input(const char* position) {
        int value;

        while (true) {
            cout << "Создать " << position << "? (1/0): ";

            if (cin >> value && (value == 0 || value == 1))
                break;

            if (cin.eof()) {
                cout << "\nВвод завершён.\n";
                return;
            }

            cin.clear();
            cin.ignore(10000, '\n');
            cout << "Введите только 0 или 1!\n";
        }

        tree.push_back(value == 1);

        if (value == 0)
            return;

        input("левую вершину");
        input("правую вершину");
    }

    // Внутренний обход с прямой разметкой
    void inorder(size_t& index, int& label) const {
        if (index >= tree.size() || !tree[index++])
            return;

        int current = ++label;

        inorder(index, label);
        cout << current << " ";
        inorder(index, label);
    }

    // Вычисление высоты дерева
    int height(size_t& index) const {
        if (index >= tree.size() || !tree[index++])
            return -1;

        int left = height(index);
        int right = height(index);

        return 1 + (left > right ? left : right);
    }

    // Подсчёт вершин на уровнях не выше 2
    int countLevels(size_t& index, int depth, int h) const {
        if (index >= tree.size() || !tree[index++])
            return 0;

        int level = h - depth;
        int count = (level <= 2) ? 1 : 0;

        count += countLevels(index, depth + 1, h);
        count += countLevels(index, depth + 1, h);

        return count;
    }

    // Изображение дерева с иерархическими отступами
    void print(size_t& index, int& label,
               vector<bool>& lines,
               bool isRoot, bool isLast,
               const char* branch) const {

        if (index >= tree.size() || !tree[index++])
            return;

        int current = ++label;

        if (!isRoot) {
            for (size_t i = 0; i < lines.size(); ++i)
                cout << (lines[i] ? "│   " : "    ");

            cout << (isLast ? "└── " : "├── ");
            cout << branch;
        }

        cout << current;

        if (isRoot)
            cout << " [КОРЕНЬ]";

        cout << "\n";

        // Проверяем наличие левого поддерева
        bool hasLeft = index < tree.size() && tree[index];

        // Для правильного определения последней ветви
        // сначала находим начало правого поддерева
        size_t rightIndex = index;
        skip(rightIndex);

        bool hasRight = rightIndex < tree.size() && tree[rightIndex];

        if (hasLeft) {
            lines.push_back(!isLast && !isRoot);

            print(index, label, lines,
                  false, !hasRight, "L: ");

            lines.pop_back();
        } else {
            skip(index);
        }

        if (hasRight) {
            lines.push_back(!isLast && !isRoot);

            print(index, label, lines,
                  false, true, "R: ");

            lines.pop_back();
        } else {
            skip(index);
        }
    }

    // Пропуск поддерева в последовательности битов
    void skip(size_t& index) const {
        if (index >= tree.size())
            return;

        if (!tree[index++])
            return;

        skip(index);
        skip(index);
    }

public:
    void build() {
        tree.clear();

        cout << "\n=== ВВОД ДЕРЕВА ===\n";
        cout << "1 - создать вершину\n";
        cout << "0 - пустое поддерево\n\n";
         cout << "можно ввести всё дерево в одну строку через пробелы\n\n";

        input("корень");
    }

    void showBits() const {
        cout << "\nПоследовательность дерева:\n";

        for (bool bit : tree)
            cout << bit << " ";

        cout << "\n";
    }

    void showTree() const {
        cout << "\n=== ИЗОБРАЖЕНИЕ ДЕРЕВА ===\n\n";

        if (tree.empty() || !tree[0]) {
            cout << "Дерево пустое.\n";
            return;
        }

        size_t index = 0;
        int label = 0;
        vector<bool> lines;

        print(index, label, lines, true, true, "");
    }

    void showInorder() const {
        cout << "\n=== ВНУТРЕННИЙ ОБХОД ===\n";

        size_t index = 0;
        int label = 0;

        inorder(index, label);

        cout << "\n";
    }

    void showResult() const {
        size_t index = 0;
        int h = height(index);

        index = 0;
        int result = countLevels(index, 0, h);

        cout << "\n=== РЕЗУЛЬТАТ ===\n";
        cout << "Высота дерева: " << h << "\n";
        cout << "Количество вершин на уровнях <= 2: "
             << result << "\n";
    }
};

int main() {
    setupRussianConsole();

    BinaryTree tree;

    cout << "Зачётная работа №3\n";
    cout << "Вариант 11\n";
    cout << "Двоичное дерево\n";
    cout << "Разметка: прямая\n";
    cout << "Обход: внутренний\n";
    cout << "Задание: количество вершин на уровне не выше 2\n";

    tree.build();

    tree.showBits();
    tree.showTree();
    tree.showInorder();
    tree.showResult();

    return 0;
}