// 手动测试入口：./p2_echo --port 8080
// 然后另一个终端：printf 'hello\n' | nc 127.0.0.1 8080

#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>

#include "p2/echo_server.hpp"

int main(int argc, char** argv) {
    std::uint16_t port = 8080;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--port" && i + 1 < argc) {
            port = static_cast<std::uint16_t>(std::stoi(argv[++i]));
        }
    }

    try {
        p2::EchoServer server(port);
        std::cout << "echo server listening on 127.0.0.1:" << server.port()
                  << " (Ctrl-C to stop)" << std::endl;
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "fatal: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
