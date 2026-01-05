#include "rcc_template.hpp"
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

template <typename T> bool compare_arr(const T &arr1, const T &arr2) {
    for (auto it1 = arr1.begin(), it2 = arr2.begin(); it1 != arr1.end() && it2 != arr2.end(); ++it1, ++it2) {
        if (*it1 != *it2) {
            cout << "\"" << *it1 << "\" != \"" << *it2 << "\"" << endl;
            return false;
        }
    }
    return true;
}

void test_ch() {
    cout << ">>>>>>>>>>>>>" << __func__ << endl;

    char c = CH(a);
    cout << c << endl;
}

void test_split() {
    cout << ">>>>>>>>>>>>>" << __func__ << endl;

    string str = "name mode size createTime accessTime";
    vector<string> v = split(str);
    vector<string> exp_v = {"name", "mode", "size", "createTime", "accessTime"};

    assert(compare_arr(v, exp_v));

    str = " name\n mode size\t\t\t createTime accessTime \n ";
    v = split(str);

    assert(compare_arr(v, exp_v));
}

void test_split_quoted() {
    cout << ">>>>>>>>>>>>>" << __func__ << endl;

    string str = "name mode size createTime accessTime";
    vector<string> v = split_quoted(str);
    vector<string> exp_v = {"name", "mode", "size", "createTime", "accessTime"};

    assert(compare_arr(v, exp_v));

    str = " name\n mode size\t\t\t createTime accessTime \n ";
    v = split_quoted(str);

    assert(compare_arr(v, exp_v));

    vector<string> exp_v2 = {"name", "mode", "size", "create Time", "access \t Time"};
    str = " name\n mode size\t\t\t \"create Time\" \"access \t Time\" \n ";
    v = split_quoted(str);

    assert(compare_arr(v, exp_v2));

    str = " name\n mode size\t\t\t 'create Time' \"access \t Time\" \n ";
    v = split_quoted(str);

    assert(compare_arr(v, exp_v2));
}

void test_traversal() {
    cout << ">>>>>>>>>>>>>" << __func__ << endl;

    // This will print every file and directory in current directory and its
    // subdirectory.
    auto callback = [&](const string &path, const string &filename, bool is_file) {
        cout << path << ", " << filename << (is_file ? "" : " [dir]") << endl;
        // return 0 to continue traversing
        // return -1 to stop traversing
        // return 1 to skip traversing the directory
        return 0;
    };
    traversal("../.", callback);
}

void test_traversal2() {
    cout << ">>>>>>>>>>>>>" << __func__ << endl;

    TRAVERSAL("../.", {
        cout << path << ", " << filename << (is_file ? "" : " [dir]") << endl;
        return 0;
    });
}

int main() {
    test_ch();
    test_split();
    // test_split_quoted();
    test_traversal();
    test_traversal2();

    return 0;
}
