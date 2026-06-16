# Project Specifications

This Unix shell is capable of running programs, redirecting input/output, and creating pipelines. More advanced features include job control and signal handling. Overall, this shell implementation will reflect actual Unix's shell implementation.

The general timeline is as follows:
- Program execution via `fork`/`exec`/`wait`
- I/O redirection via `>`, `<`, `>>` via `dup2`
- Pipelined execution
- Signal handling
- Job control

## Program Execution

A command line is composed of a command name followed by a list of arguments to the command. The shell parses the command line by separating the command name and the arguments into separate strings. The shell then searches for a file with the specified command name, bringing the command into memory and executing once found. In Unix's implementation, general use commands such as `echo` and `cat` are found within a system directory containing all these commands, specifically in `/bin` directories. For example, the `root` directory is a system directory that Unix specifies.

The shell's lifetime consists of the following: launching and setup with configurations, reading STDIN in loop, cleanup and termination.

Whenever the shell executes a program, it generally inherits three open files with file descriptors 0, 1, and 2 from the shell. File 0 represents STDIN, file 1 STDOUT, and file 2 STDERR. Exceptions to this case occur when the user specifies changes to STDIN, STDOUT, and STDERR behavior such as specifying output location via `program > output.txt`. Programs executed by the shell are child processes of the shell.

For the purposes of this project, this shell will resemble the core functionalities of the Unix shell implementation. More specifically, `main()` will initialize the shell configurations, run a forever loop, and cleanup on termination. General use commands such as `echo`, `cat`, `ls`, etc. will reside within a local `/bin` directory. The shell will first search its working directory for the command it is looking, otherwise it will check `/bin`. If found, the shell will create a child process for the program and intialize its memory, such as its file descriptors.

## I/O Redirection

As implied by the previous section, I/O redirection occurs when the file descriptors for STDIN, STDOUT, and STDERR are remapped. Unix uses `dup2(int oldfd, int newfd)` (found at `man 2 dup2`) for I/O redirection to remap `oldfd` to `newfd` file descriptor. For example, to execute `ls > output.txt`, the shell would have to remap the STDOUT file descriptor of `ls` to the file descriptor for `output.txt`.

This project will implement I/O redirection similar to Unix by using `dup2()`.

## Pipelined Execution

Unix implements pipelined execution by using `pipe(int pipefd[2])` (found at `man 2 pipe`) to generate a file descriptor that allows inter-process communication and an inter-process channel called a pipe. This inter-process channel is passed from parent to child using `fork()`.

This project will utilize `pipe()` to implement inter-process communication in the likeness of Unix's implementation.

## Signal Handling

TODO

## Job Control

TODO
