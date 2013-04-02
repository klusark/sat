#include <cstdio>
#include <vector>
#include <string>
#include <iostream>
#include <cmath>

int n = 0;
int s = 0;

struct Pos{
    int x;
    int y;
    int v;
};

std::vector<Pos> pos;

int main()
{
    {
        std::vector<char> nums;
        int num = -1;
        while (!std::cin.eof()) {
            num = -1;
            std::string text;
            std::cin >> text;
            if (text != ".") {
                sscanf(text.c_str(), "%d", &num);
            }
            nums.push_back(num);
        }
        int size = nums.size() - 1;
        s = sqrt(size);
        n = sqrt(s);
        if ((n * n) != s || (s * s) != size) {
            printf("Your board is sized incorrectly\n");
            return 1;
        }
        int x = 0;
        int y = 0;
        for (int i = 0; i < size; ++i) {
            int val = nums[i];
            if (val != -1) {
                Pos p;
                p.x = x;
                p.y = y;
                p.v = val;
                pos.push_back(p);
            }
            ++x;
            if ((i + 1) % s == 0) {
                ++y;
                x = 0;
            }
        }
    }

    printf("TYPE INT [ 1.. %d]\n", s);
    printf("PREDICATE Filled\n");
    for (const Pos &p : pos) {
        printf("(%d, %d, %d)\n", p.x, p.y, p.v);
    }
    printf("FUNCTION N\n");
    printf("( : %d )\n", n);
}
