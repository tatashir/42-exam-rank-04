#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>

int err(char *s)
{
    while (*s)
        write(2, s++, 1);
    return 1;
}

int cd(char **argv, int i)
{
    if (i != 2)
        return err("error: cd: bad arguments\n");
    else if (chdir(argv[1]) == -1)
        return err("error: cd: cannot change directory to "), err(argv[1]), err("\n");
    return 0;
}

int exec(char **argv, char **envp, int i)
{
    int fd[2];
    int status;
    int hasPipe = argv[i] && !strcmp(argv[i], "|");
    if (hasPipe && pipe(fd) == -1)
        return err("error: fatal\n");
    int pid = fork();
    if (!pid)
    {
        argv[i] = 0;
        if (hasPipe && (dup2(fd[1], 1) == -1 || close(fd[0]) == -1 || close(fd[1]) == -1))
            return err("error: fatal\n");
        execve(*argv, argv, envp);
        return err("error: cannot execute "), err(*argv), err("\n");
    }
    waitpid(pid, &status, 0);
    if (hasPipe && (dup2(fd[0], 0) == -1 || close(fd[0]) == -1 || close(fd[1]) == -1))
        return err("error: fatal\n");
    return WIFEXITED(status) && WEXITSTATUS(status);
}

int main(int argc, char **argv, char **envp)
{
    int i = 0;
    int status = 0;
    if (1 < argc)
    {
        while (argv[i] && argv[++i])
        {
            argv += i;
            i = 0;
            while (argv[i] && strcmp(argv[i], "|") && strcmp(argv[i], ";"))
                i++;
            if (!strcmp(*argv, "cd"))
                status = cd(argv, i);
            else if (i)
                status = exec(argv, envp, i);
        }
    }
    return status;
}

//strcmp 1001