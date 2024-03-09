#include "templates/commons.hpp"

#include <iostream>
using namespace std;

int no_of_test_cases = -1;

int solve() {

    return 0;
}

int main() {
    ios::sync_with_stdio(false);
    if (no_of_test_cases < 0) cin >> no_of_test_cases;
    for (int i = 0; i < no_of_test_cases; ++i) {
        // cout << "Case #" << i + 1 << ": ";
        if (int return_code = solve(); return_code != 0) {
            return return_code;
        }
    }
    return 0;
}
