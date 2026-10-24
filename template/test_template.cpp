#include <iostream>
#include <cstring>

using namespace std;

template <typename T>
int compare(const T& a, const T& b) {
    cout << "This is func A." << endl;
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

template <>
int compare<const char*>(const char* const& a, const char* const& b) {
    cout << "This is func B." << endl;
    return strcmp(a, b);
}

// 普通函数重载，优先级高于模板特化
int compare(const char* a, const char* b) {
    cout << "This is func C." << endl;
    return strcmp(a, b);
}

int main()
{
	const char* left = "abc";
	const char* right = "def";
    compare(left, right);

}
