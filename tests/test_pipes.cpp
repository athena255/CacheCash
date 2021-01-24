//
// Created by sage on 1/19/21.
//
#include <catch2/catch.hpp>
#include <stdafx.h>
#include <wait.h>

#define READ 0
#define WRITE 1

TEST_CASE("pipe()", "[.pipes]")
{
    int fd[2];
    pipe(fd);

    // write to pipe
    char const *msg = "Hello World!";
    write(fd[WRITE], msg, strlen(msg)+1);
    close(fd[WRITE]);

    // read from pipe
    char buf[128];
    auto n_bytes = read(fd[READ], buf, sizeof(buf));

    REQUIRE(n_bytes == strlen(msg)+1);

    close(fd[READ]);
    close(fd[WRITE]);
}

TEST_CASE("parent receive from child", "[.pipes]")
{
    pid_t child;
    int fd[2];
    pipe(fd);
    char const *msg = "Hello World!";

    child = fork();
    if (child == 0)
    {
        // Child wants to send data
        close(fd[READ]);
        FILE *s = fdopen(fd[WRITE], "w");
        fprintf(s, "%s", msg);
        fflush(s);
//        write(fd[WRITE], msg, strlen(msg)+1);
        close(fd[WRITE]);
    }
    else
    {
        // Parent wants to receive data
        close(fd[WRITE]);
        char buf[128];
        auto n_bytes = read(fd[READ], buf, sizeof(buf));
        REQUIRE(n_bytes == strlen(msg));
        close(fd[READ]);
    }
}

TEST_CASE("child receive from parent", "[.pipes]")
{
    pid_t child;
    int fd[2];
    pipe(fd);
    char const *msg = "Hello World!";

    child = fork();
    if (child == 0)
    {
        // Child wants to receive data
        close(fd[WRITE]);
        char buf[128];
        auto n_bytes = read(fd[READ], buf, sizeof(buf));
        REQUIRE(n_bytes == strlen(msg)+1);
        close(fd[READ]);
    }
    else
    {
        // Parent wants to send data
        close(fd[READ]);
        write(fd[WRITE], msg, strlen(msg)+1);
        close(fd[WRITE]);
    }
}

TEST_CASE("parent into child stdin", "[.pipes]")
{

    auto out_fp = popen("./Spec1Victim", "w");
    fprintf(out_fp, "%llu\n", 1);
    fprintf(out_fp, "%llu\n", 2);
    pclose(out_fp);
}

TEST_CASE("duplex", "[pipes]")
{
    pid_t child;
    int fd[2];
    int tx[2];
    pipe(fd);
    pipe(tx);

    child = fork();
    if (child == 0)
    {
        // Child wants to send data
        close(STDOUT_FILENO);
        dup(fd[WRITE]);

        close(STDIN_FILENO);
        dup(tx[READ]);

        close(fd[READ]);
        close(tx[WRITE]);
        close(tx[READ]);
        close(fd[WRITE]);
        execlp("./Spec1Victim", NULL, NULL);

    }
    else
    {
        // Parent wants to receive data
        close(fd[WRITE]);
        char buf[0x100];

        close(tx[READ]);
        FILE *in_stream = fdopen(tx[WRITE], "w");
        fprintf(in_stream, "%llu\n", 1);
        fflush(in_stream);

        write(tx[WRITE], "2\n", 2);
        write(tx[WRITE], "3\n", 2);

        close(tx[WRITE]);
        read(fd[READ], buf, sizeof(buf));
        wait(0);
        printf("this: %s !", buf);
        close(fd[READ]);
    }
}
