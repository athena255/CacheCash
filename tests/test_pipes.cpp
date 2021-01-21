//
// Created by sage on 1/19/21.
//
#include <catch2/catch.hpp>
#include <stdafx.h>
#include <unistd.h>

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
        write(fd[WRITE], msg, strlen(msg)+1);
        close(fd[WRITE]);
    }
    else
    {
        // Parent wants to receive data
        close(fd[WRITE]);
        char buf[128];
        auto n_bytes = read(fd[READ], buf, sizeof(buf));
        REQUIRE(n_bytes == strlen(msg)+1);
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

TEST_CASE("parent into child stdin", "[pipes]")
{

    auto out_fp = popen("./Spec1Victim", "w");
    fprintf(out_fp, "%llu\n", 1);
    fprintf(out_fp, "%llu\n", 2);
    pclose(out_fp);
}


