#include "helper.cpp"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

SOCKET client_sockets[10];

int extractID(const char msg[1000], char modifiedMsg[1000]) {
    int number = 0;
    int i = 0;
    int j = 0;

    // Skip any leading whitespace
    while (isspace(msg[i]))
        i++;

    // Extract the number
    while (isdigit(msg[i])) {
        number = number * 10 + (msg[i] - '0');
        i++;
    }

    // Copy the modified message without the number
    while (msg[i] != '\0') {
        modifiedMsg[j] = msg[i];
        i++;
        j++;
    }
    modifiedMsg[j] = '\0';

    return number;
}

void thread_to_recv(struct thread_data td)
{
    SOCKET s = td.s; //sender socket
    char id[10]; sprintf(id, "%d", td.id); //sender socket id
    SOCKET receiver_socket;
    int receiver_id;
    char msg_recv[1000];
    char msg_to_send[1000];

    strcpy(msg_recv,"");

    // send the clients id to the client
    char send_id[30]; strcpy(send_id, "YOUR ID IS: "); strcat(send_id, id);
    send_to_socket(s, send_id);


    while(strcmp(msg_recv,"bye"))
    {
        strcpy(msg_to_send, "");
        strcat(msg_to_send, id); strcat(msg_to_send, ": ");

        int msg_len = receive_from_socket(s, msg_recv, 1000);

        if(msg_len>0){
            //send the msg to the destined client socket
            receiver_id = extractID(msg_recv, msg_recv);
            if(receiver_id ==0) {
                printf("Receiver ID not specified by sender. Discarding the request.\n");
            }
            else{
                receiver_socket = client_sockets[receiver_id - 100];
                strcat(msg_to_send, msg_recv);

                bool success = send_to_socket(receiver_socket, msg_to_send);
                if(!success) printf("Send failed.\n\n");
            }
        }
    }
}



int main(int argc , char *argv[])
{
    bool success = init_networking();
    if(!success) return 0;

    SOCKET server_socket = create_socket();
    if(!server_socket) return 0;

    success = bind_socket(server_socket, 0, 8888);
    if(!success) return 0;

    listen_for_connections(server_socket, 1);

    int i = 0; //keeps count of the number of connections
    while(true){
        //Accept and incoming connection
        puts("Waiting for incoming connections...");
        client_sockets[i] = accept_connection(server_socket);
        printf("\n connection accepted for socket %d", i);

        //create thread to handle communication for this client
        struct thread_data td;
        td.s = client_sockets[i];
        td.id = 100 + i;
        i++;

        //create_socket_thread(thread_to_send, td);
        create_socket_thread(thread_to_recv, td);
    }
	//go to sleep
    sleep_for_ever();

    //finally process cleanup job
    closesocket(server_socket);
    for(int j=0; j<i; j++) closesocket(client_sockets[j]);


    WSACleanup();
	return 0;
}



