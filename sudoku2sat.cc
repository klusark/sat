#include <cstdio>
#include <vector>
#include <string>
#include <iostream>
#include <cmath>

int n = 0;
int s = 0;

std::vector<int *> lines;
std::vector<int> done;

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
		for (int k = 0; k < s; ++k) {
			int *l = new int[s];
			int i = 0;
			for (*ax = 0; *ax < s; *ax += 1) {
				l[i++] = (y * s * s + x * s + k) + 1;
			}
			lines.push_back(l);
		}
	}
}

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
                done.push_back(y*s*s + x * s + (val));
            }
            ++x;
            if ((i + 1) % s == 0) {
                ++y;
                x = 0;
            }
        }
    }

	//rows
	makeline(true);

	//columns
	makeline(false);

	//squares
	for (int y = 0; y < n; ++y) {
		for (int x = 0; x < n; ++x) {
			for (int k = 0; k < s; ++k) {
				int *l = new int[s];
				int m = 0;
				for (int j = 0; j < n; ++j) {
					for (int i = 0; i < n; ++i) {
						l[m++] = ((y * n + j) * s * s + (x * n + i) * s + k) + 1;
					}
				}
				lines.push_back(l);
			}
		}
	}

	for (int y = 0; y < s; ++y) {
		for (int x = 0; x < s; ++x) {
			int *l = new int[s];
			int i = 0;
			for (int k = 0; k < s; ++k) {
				l[i++] = (y * s * s + x * s + k) + 1;
			}
			lines.push_back(l);
		}
	}

    int mul = 0;
    for (int i = 0; i < s; ++i) {
        mul += i;
    }
	int size = lines.size();
	printf("p cnf %d %d\n", s * s * s, size + size * mul + done.size());
	for (int i = 0; i < size; ++i) {
		for (int x = 0; x < s; ++x) {
			printf("%d ", lines[i][x]);
		}
		printf("0\n");
		for (int x = 0; x < s; ++x) {
			for (int y = x + 1; y < s; ++y) {
				printf("-%d -%d 0\n", lines[i][x], lines[i][y]);
			}
		}
		delete[] lines[i];
	}
	size = done.size();
	for (int i = 0; i < size; ++i) {
		printf("%d 0\n", done[i]);
	}
}
