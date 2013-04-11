#include <iostream>
#include <string>

using namespace std;

int main() {
    std::string order;
    cin >> order;
    int c, b;
    cin >> c;
    cin >> b;
    int i = 1;
    while (!cin.eof()) {
        int a = -52;
        cin >> a;
        if (a == -52) {
            return 0;
        }
        if (a == -1) {
            cout << " . ";
        } else {
            printf("%02d ", (a+1));
        }
        ++i;
        if ((i % (c + 1)) == 0) {
            cout << endl;
            i = 1;
        }
    }
}
