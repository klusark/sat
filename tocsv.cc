#include <iostream>
#include <cstdio>

using namespace std;

int main(int argc, const char **argv)
{
    int columns = 7;
    if (argc == 2) {
        sscanf(argv[1], "%d", &columns);
    }

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
