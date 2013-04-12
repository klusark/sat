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
    int line = 0;
    cout << line << ",";
    while (!cin.eof()) {
        cin.getline(buffer, 1024);
        if (cin.eof()) {
            break;
        }
        if (buffer[0] == 'C' || buffer[0] == '*') {
            continue;
        }
        cout << buffer;
        ++column;
        if (column == columns) {
            ++line;
            cout << endl << line << ",";
            column = 0;
        } else {
            cout << ",";
        }
    }
    cout << endl;
}
