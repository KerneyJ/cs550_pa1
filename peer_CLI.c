#include <stdio.h> 

int launch_CLI() {
    //Welcome
    printf("\n-------------------------------------------------\n");
    printf("Howdy partner 🤠, ready to pirate some 💿💿💿?");
    printf("\n-------------------------------------------------\n\n");
  
    
    int is_running = 1;
    int user_input;
    while (is_running) {
        //Main menu
        
        printf("\nSelect a menu item:\n");
        printf("--------------------\n");
        printf("[1] 🤝 Register as a new user (and file directory)\n");
        printf("[2] 💰 Register a file\n");
        printf("[3] 🔎 Search for a file on the server index\n");
        printf("[4] 📦 Request a file to download from the network\n");
        printf("[9] 👋 quit (and log off the network).\n");

        scanf("%d", &user_input);
        if (user_input == 9) {
            printf("Aye Matey, Farewell! 👋\n\n");
            is_running = 0;
        }
        else if (user_input == 1) {
            printf("\nRegistering new user...\n");
        }
        else if (user_input == 2) {
            printf("Registering a new file...\n");
        }
        else if (user_input == 3) {
            printf("Searching for a file on the server...\n");
        }
        else if (user_input ==4) {
            printf("Requesting a file to download...\n");
        }
    }
    return 0;
}



int main(int argc, char const *argv[])
{
    launch_CLI();
    return 0;
}


