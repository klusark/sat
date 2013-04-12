#include <cstdio>
#include <vector>
#include <string>
#include <iostream>
#include <cmath>

int n = 0;
int s = 0;
int bits = 4;

std::vector<int *> lines;
std::vector<int> done;

void permute(std::vector<int> p, int l)
{
    if (l == bits) {
        for (int i = 0; i < bits; ++i) {
            printf("%d %d ", p[i*2], p[i*2+1]);
        }
        printf("0\n");
        return;
    }

    permute(p, l + 1);
    p[l*2] *= -1;
    p[l*2 + 1] *= -1;
    permute(p, l + 1);
}


void makeline(bool row)
{
	int x = 0;
	int y = 0;

	int *ay = &x;
	int *ax = &y;
	if (row) {
		ay = &y;
		ax = &x;
	}

    for (*ay = 0; *ay < s; *ay += 1) {
        for (*ax = 0; *ax < s; *ax += 1) {
            for (int i = *ax + 1; i < s; ++i) {
                std::vector<int> p;
                for (int j = 0; j < bits; ++j) {
                    p.push_back((y*s*bits)+(x*bits)+j + 1);
                    if (row) {
                        p.push_back((y*s*bits)+((i*bits)+j) + 1);
                    } else {
                        p.push_back((i*s*bits)+((x*bits)+j) + 1);
                    }
                }
                permute(p, 0);
            }
        }
    }
}

void dosquare(int x, int y)
{
    x *= n;
    y *= n;
    for (int k = 0; k < s; ++k) {
        for (int i = k + 1; i < s; ++i) {
            std::vector<int> p;
            for (int j = 0; j < bits; ++j) {
                int x2 = k%n;
                int y2 = k/n;
                int x3 = i%n;
                int y3 = i/n;
                p.push_back(((y + y2)*s*bits)+((x+x2)*bits)+j + 1);
                p.push_back(((y+y3)*s*bits)+(((x+x3)*bits)+j) + 1);
            }
            permute(p, 0);
        }
    }
}

int main()
{
    int numnondot = 0;
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
                numnondot += 1;
            }
            done.push_back(val);
            ++x;
            if ((i + 1) % s == 0) {
                ++y;
                x = 0;
            }
        }
    }

    int numvar = bits * s * s;
    int numc = numnondot*4 + 92160;

    printf("p cnf %d %d\n", numvar, numc);

    makeline(true);

    makeline(false);

	for (int y = 0; y < n; ++y) {
		for (int x = 0; x < n; ++x) {
            dosquare(x, y);

		}
	}

	for (int i = 0; i < done.size(); ++i) {
        if (done[i] == -1) {
            continue;
        }
        for (int j = 0; j < bits; ++j) {
            if ((1 << j) & done[i]) {
		        printf("%d 0\n", (i*4+j) + 1);
            } else {
		        printf("-%d 0\n", (i*4+j) + 1);
            }
        }
	}

}
