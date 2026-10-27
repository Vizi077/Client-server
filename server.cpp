#include <cstdio> 
#include <cstdlib> 
#include <cstring>
#include <csignal>

#include <iostream>

#include <sys/types.h>      // базовые типы (ssize_t и т.п.)
#include <sys/socket.h>     // socket(), bind(), listen(), accept()
#include <netinet/in.h>     // struct sockaddr_in, htons()
#include <arpa/inet.h>      // htonl(), inet_*()
#include <unistd.h>         // close(), read(), write()

#define PORT 1100

int bind_socket(int fd,const sockaddr_in& addr)
{
    if(bind(fd, reinterpret_cast<const sockaddr*> (&addr), sizeof(addr)) == -1)
    {
        std::perror("Bind!");
        return -1;
    }
    
    return 0;
}

// Считывание ввода текста от пользователя и вывод его в консоль сервера
void handle_client(int clientFD)
{
    char buf[1024];
    ssize_t n = read(clientFD, buf, sizeof(buf) - 1);
    if (n > 0) 
    {
        buf[n] = '\0';
        std::cout << "Message from Client: " << buf << "\n";

        // Привет от сервера
        const char reply[] = "Hello from Server!\n";
        ssize_t sent = write(clientFD, reply, sizeof(reply) - 1);
        if (sent == -1) 
        {
            std::perror("Can't Write!");
        }

    } 
        
    else if (n == 0) 
    {
        std::cout << "Client closed connection...\n";
    } 

    else 
    {
        std::perror("Can't Read!");
    }
    
}

// Закрытие соединения с клиентом
void client_close(int clientFD)
{
    shutdown(clientFD, SHUT_RDWR);
    close(clientFD);
}


int main()
{
    std::signal(SIGPIPE, SIG_IGN);


    int listenFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (listenFD == -1)
    {
        std::perror("Bad Socket!");
        return EXIT_FAILURE;
    }



    // Разрешить повторно использовать локальный адрес (порт)
    int yes = 1;
    if (setsockopt(listenFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) 
    {
        std::perror("setsockopt");
        close(listenFD);
        return EXIT_FAILURE;
    }



    sockaddr_in addr{};                         // Создаём объект addr из структуры, описывающей IPv4-адрес
    addr.sin_family = AF_INET;                  // Каким протоколом пользоваться. IPv4
    addr.sin_port = htons(PORT);                // Порт, на котором сервер будет слушать. PORT = 1100
    addr.sin_addr.s_addr = htonl(INADDR_ANY);   // IP-адрес, на котором слушать
    if (bind_socket(listenFD, addr) == -1)
    {
        close(listenFD);
        return EXIT_FAILURE;
    }



    // Прослушиваем порт (для TCP)
    if(listen(listenFD,256) == -1) // 256 - до 256 подключений в очереди
    {
        std::perror("Can't listen!");
        close(listenFD);
        return EXIT_FAILURE;
    }



    std::cout<<"Server listen port "<<PORT<<"\n";



    for(;;)
    {
        int clientFD = accept(listenFD,nullptr,nullptr);
        if(clientFD < 0)
        {
            std::perror("Can't accept!");
            continue;
        }

        // Работа с клиентом------------------------------------
        handle_client(clientFD);


        // Завершение работы
        
        
        client_close(clientFD);

    }


    return 0;
}