#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


#define PORT 1100

// Читает одну команду от пользователя.
// Возвращает false, если пользователь ввёл "quit" или нажал Ctrl+D.
// При успехе записывает команду в out (без '\n').
bool read_command(std::string& out)
{
    std::cout << "> " << std::flush;

    std::string line;
    if (!std::getline(std::cin, line))
        return false;   // EOF (Ctrl+D)

    // Обрезаем \r\n на всякий случай (например, при вставке из Windows)
    while (!line.empty() && (line.back() == '\n' || line.back() == '\r'))
        line.pop_back();

    if (line == "quit")
        return false;

    if (line.empty())
        return true;    // пустой Enter — пропускаем, но не выходим

    out = line;
    return true;
}




int main()
{
    std::signal(SIGPIPE, SIG_IGN);   // на случай, если сервер закроет соединение

    int sockFD = socket(AF_INET,SOCK_STREAM,0);
    if (sockFD == -1) {std::perror("Bad socket"); return EXIT_FAILURE;}

    // Адрес сервера
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    inet_pton(AF_INET,"127.0.0.1", &addr.sin_addr);

    // Подключение
    if(connect(sockFD, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) == -1)
    {
        std::perror("Can't connected");
        close(sockFD);
        return EXIT_FAILURE;
    }

    std::cout << "Connected to 127.0.0.1:" << PORT << "\n";
    std::cout << "Type command (ping / hello / quit):\n";

    std::string cmd;
    while (read_command(cmd))               
    {
        if (cmd.empty()) continue;          

        cmd += "\n";                        
        ssize_t sent = write(sockFD, cmd.data(), cmd.size());
        if (sent == -1) { std::perror("write"); break; }

        char buf[1024];
        ssize_t n = read(sockFD, buf, sizeof(buf) - 1);
        if (n > 0)
        {
            buf[n] = '\0';
            std::cout << "Server: " << buf;
        }
        else if (n == 0)
        {
            std::cout << "Server closed connection\n";
            break;
        }
        else
        {
            std::perror("read");
            break;
        }
    }

    close(sockFD);
    return 0;
    
    




}