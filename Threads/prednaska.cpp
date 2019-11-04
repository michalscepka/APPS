#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

struct TaskDes
{
    int from, len;
    char *str;
};

void str_up(char* str, int from, int len)
{
    for(int i = 0; i < len; i++)
    {
        str[from + i] = toupper(str[from + i]);
    }
}

void *vlakno(void* arg)
{
    TaskDes td = *(TaskDes*)arg;
    str_up(td.str, td.from, td.len);
    return nullptr;
}

int main()
{
    char ret[] = "Bla bla blaBla bla blaBla bla blaBla bla bla";

    int vlaken = 2;
    pthread_t pt[vlaken];
    TaskDes td1 = { 0, (int)strlen(ret) / 2, ret };
    TaskDes td2 = { (int)strlen(ret) / 2, (int)strlen(ret) - (int)strlen(ret) / 2, ret };
    
    pthread_create(&pt[0], nullptr, vlakno, &td1;
    pthread_create(&pt[1], nullptr, vlakno, &td2;
    
    pthread_join(&pt[0], nullptr);
    pthread_join(&pt[1], nullptr);

    /*int param[vlaken];
    for(int i = 0; i < vlaken; i++)
    {
        param[i] = i;
                                            //argument
        pthread_create(&pt[i], nullptr, vlakno, &param[i];
    }

    for(int i = 0; i < vlaken; i++)
    {
        pthread_join(&pt[i], nullptr);
    }*/

    printf("Vysledek %s\n", ret);

    //testovani funkce
    //vlakno(nullptr);
    //vlakno(nullptr);

    printf("Main konci...\n");
}


//ECLIPSE NA LINUXU
//import Linux GCC

//2 vlakna cas jednoho vlakna *4
//4 vlakna cas jednoho vlakna *16
//6 vlaken cas jednoho vlakna *36
