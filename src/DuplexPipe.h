//
// Created by sage on 1/22/21.
//

#ifndef CACHECASH_DUPLEXPIPE_H
#define CACHECASH_DUPLEXPIPE_H

#include <poll.h>
#include <cstdarg>

class DuplexPipe
{
public:
    static constexpr int READ = 0;
    static constexpr int WRITE = 1;
    static constexpr size_t BUF_SIZE = MAX_RX_BUF;
    DuplexPipe(char const *cmd, char * const *args)
    {
        pipe(rx);
        pipe(tx);
        if (fork() == 0)
        {
            close(STDOUT_FILENO);
            dup(rx[WRITE]);

            close(STDIN_FILENO);
            dup(tx[READ]);

            close(rx[READ]);
            close(tx[WRITE]);
            close(tx[READ]);
            close(rx[WRITE]);
            execve(cmd, args, NULL);
        }
        else
        {
            close(rx[WRITE]);
            close(tx[READ]);
            in_stream = fdopen(tx[WRITE], "w");

            pfd[READ].fd = rx[READ];
            pfd[READ].events = POLLIN;

            pfd[WRITE].fd = tx[WRITE];
            pfd[WRITE].events = POLLOUT;
        }
    }
    ~DuplexPipe()
    {
        close(tx[WRITE]);
        close(rx[READ]);
    }

    inline void send(char const * const msg)
    {
        if (poll(&pfd[WRITE], 1, -1) == 1)
            write(tx[WRITE], msg, strlen(msg)+1);
    }

    inline void send(char * const buf, size_t buf_size)
    {
        if (poll(&pfd[WRITE], 1, -1) == 1)
            write(tx[WRITE], buf, buf_size);
    }

    inline void fmt_send(const char *format, ...)
    {
        if (poll(&pfd[WRITE], 1, -1) == 1)
        {
            va_list args;
            va_start(args, format);
            vfprintf(in_stream, format, args);
            va_end(args);
            fflush(in_stream);
        }
    }

    inline size_t receive(char *buf, size_t buf_size)
    {
        return read(rx[READ], buf, buf_size);
    }

    inline size_t receive_str()
    {
        auto n_bytes = read(rx[READ], rx_buf, BUF_SIZE-1);
        rx_buf[n_bytes] = 0;
        return n_bytes;
    }

    // This function will wait until there is something in the pipe
    inline void wait_clear_rx()
    {
        auto n_bytes = read(rx[READ], rx_buf, BUF_SIZE-1);
        rx_buf[n_bytes] = 0;
        D("%s", rx_buf);
        clear_rx();
    }

    // This function will return immediately if there is nothing to read in the pipe
    inline void clear_rx()
    {
        size_t n_bytes;
        while(poll(&pfd[READ], 1, 0) == 1)
        {
            n_bytes = read(rx[READ], rx_buf, BUF_SIZE-1);
            rx_buf[n_bytes] = 0;
            D("%s", rx_buf);
        }
    }

    void print_rx_buf()
    {
        printf("\n============ BEG ============\n%s\n=============================\n", rx_buf);
    }

    char rx_buf[BUF_SIZE];

private:
    int rx[2];
    int tx[2];
    struct pollfd pfd[2]{};
    FILE *in_stream;
};
#endif //CACHECASH_DUPLEXPIPE_H
