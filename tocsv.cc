#include <iostream>

using namespace std;

int main()
{
    int columns = 7;

    int column = 0;

    char buffer[1024];
    while (!cin.eof()) {
        cin.getline(buffer, 1024);
        if (cin.eof()) {
            break;
        }
        if (buffer[0] == 'C') {
            continue;
        }
        cout << buffer;
        ++column;
        if (column == columns) {
            cout << endl;
            column = 0;
        } else {
            cout << ",";
        }
    }
    cout << endl;
}
