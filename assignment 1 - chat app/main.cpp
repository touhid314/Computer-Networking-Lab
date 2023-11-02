#include<stdio.h>

#include "helper.cpp"

void some_func(SOCKET s)
{
    printf("Inside user function.\n\n");
}

int main(void)
{
    SOCKET s;
    create_thread(some_func, s);
}
