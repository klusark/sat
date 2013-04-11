#include <iostream>

using namespace std;

int main() {
    char a[256];
    cin.getline(a, 256);

    while (!cin.eof()) {
        int a = 0;
        cin >> a;
        if (a == -1) {
            cout << ". "
        } else {
            cout << a+1 << " ";
        }
    }
}
