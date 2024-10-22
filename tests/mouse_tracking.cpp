#include <iostream>
#include <unistd.h>
#include <termios.h>

void enableMouseTracking() {
    // Enable mouse tracking
    std::cout << "\033[?1000h" << std::flush; // Enable basic mouse tracking
}

void disableMouseTracking() {
    // Disable mouse tracking
    std::cout << "\033[?1000l" << std::flush; // Disable basic mouse tracking
}

void setRawMode() {
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

int main() {
    setRawMode();
    enableMouseTracking();

    char buf[32];
    while (true) {
        ssize_t n = read(STDIN_FILENO, buf, sizeof(buf));
        if (n > 0) {
            buf[n] = '\0';
            //std::cout << "Input: " << buf << std::endl;

            if (buf[0] == 'q' || buf[0] == 'Q') {
                disableMouseTracking();
                exit(0);
                break;
            }

            // Example of recognizing mouse click
            if (buf[0] == '\033' && buf[1] == '[' && buf[2] == 'M') {
                int button = buf[3] - 32;
                int x = buf[4] - 32;
                int y = buf[5] - 32;
                std::cout << "Mouse click at (" << x << ", " << y << ") with button " << button << std::endl;
            } else {
                std::cout << "key: " << buf[0] << ".\n";
            }
        }
    }

    disableMouseTracking();
    return 0;
}
