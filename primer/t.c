#include <stdio.h>
#include <unistd.h>
#include <conio.h>
int main(){
    for(int i=0;i<10;i++){
        char ch = getch();
        if(ch == 127){
            printf("\b \b");
            fflush(stdout);
            continue;
        } else {
            printf("%c", ch);
            fflush(stdout);
        }
        printf("next");

    }
}