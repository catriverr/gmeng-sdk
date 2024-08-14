
#include <iostream>
#include <string>

static std::string colors[] = {
    "\x1B[39m", "\x1B[34m", "\x1B[32m", "\x1B[36m",
    "\x1B[31m", "\x1B[35m", "\x1B[33m", "\x1B[30m", "\x1B[37m"
};

void printHello(int index) {
    if (index < 0 || index >= sizeof(colors) / sizeof(colors[0])) {
        std::cout << "Invalid color index." << std::endl;
        return;
    }

    std::cout << colors[index] << "Hello there" << colors[0] << " - Index: " << index << std::endl;
}

int main() {
    for (int i = 0; i < sizeof(colors) / sizeof(colors[0]); ++i) {
        printHello(i);
    }

    return 0;
}
