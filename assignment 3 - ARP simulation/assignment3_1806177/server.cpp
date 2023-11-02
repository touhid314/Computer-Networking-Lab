#include "helper.cpp"

struct mac_address_entry
{
    int port;    // port number (only used in switch)
                 // you can define additional parameters if needed to send to threads
    SOCKET s;    // Storing the client socket to send frame
    char mac[7]; // eg- ab:bc:cd:ef:10:12
};

mac_address_entry table[255]; // take a large table
int table_size = 0;

thread_data clients[255];     // Store all clients here
int client_size = 0;

int current_free_port = 0;

void *command_listener(void *arg)
{
    char command[100];

    while (true)
    {
        gets(command);

        if (strcmp(command, "exit") == 0) // exiting
            exit(0);
        // Add your code here
        if(strcmp(command, "sh mac address-table")==0){
            //print the arp table
            if(table_size == 0){
                printf("MAC address table is empty\n\n");
            }else{
                printf("MAC address table:\n");
                for(int i=0; i<table_size; i++){
                    printf("Entry %d, Port: %d ", i+1, table[i].port);
                    printf(" MAC:");
                    print_mac(table[i].mac);
                    puts("\n");
                }
            }

            puts("\n");
            continue;
        }

        if(strcmp(command, "clear mac address-table") == 0){
            table_size = 0;
            printf("MAC table deleted\n\n");
            continue;
        }


        printf("\n\n\n");
    }
}

// return receiver socket based on given mac address.
SOCKET get_receivers_socket(mac_address_entry *table, int table_size, char *receiver_mac)
{
    int i;
    for (i = 0; i < table_size; i++)
        if (!strcmp(table[i].mac, receiver_mac))
            return table[i].s;

    return 0;
}

void thread_to_recv(thread_data data)
{
    puts("\n\n\n");

    // take a ethernet frame pointer and allocate memory. Allocation memory is important here.
    ethernet_frame *frame = (ethernet_frame *)malloc(sizeof(ethernet_frame));
    char mac_source[7];
    char mac_dest[7];

    while (true)
    {
        puts("\n\n");
        int msg_len = receive_ethernet_frame(data.s, frame);
        if (msg_len < 0){
            printf("ethernet frame receive failed for the client in server\n");
            break; // once send or receive fails, please return;
        }
        if (msg_len > 0)
        {
            // Add your code here

            // ------------------------ handling received ethernet frame
            // extract source mac and store it in the MAC table along with assigned port number and socket
            strcpy(mac_source, frame-> mac_source);
            printf("received frame from MAC: ");
            print_mac(mac_source);
            puts("\n");

                // add it to the table if it doen't already exists
            int already_exists = 0;
            for(int i=0; i<table_size; i++){
                if(strcmp(mac_source, table[i].mac) == 0){
                    already_exists = 1;
                }
            }
            if(already_exists == 0){
                //make a mac address entry variable
                struct mac_address_entry new_table_entry;
                new_table_entry.port = current_free_port;
                current_free_port++;
                new_table_entry.s = data.s;
                strcpy(new_table_entry.mac, mac_source);


                table[table_size] = new_table_entry;
                table_size++;

                    //confirming
                printf("added new entry to the table\n");
                printf("port: %d, mac: ", table[table_size-1].port);
                print_mac(table[table_size-1].mac);
                puts("\n");
                }


            // extract destination mac address
            strcpy(mac_dest, frame-> mac_destination);

            char broad_mac[7];
            generate_broadcast_mac(broad_mac);

            if (strcmp(mac_dest, broad_mac)==0){
                // dest mac add = FFFF, broadcast to all devices except the sender
                printf("dest mac = broadcast mac \n\n");
                for(int i = 0; i<client_size; i++){
                    if(clients[i].s != data.s){
                        send_ethernet_frame(clients[i].s, frame);
                        printf("sent ethernet frame to client %d\n\n", i);
                    }
                }

            }else{
                printf("dest mac address is not broadcast\n\n");
                // dest mac add != FFFF,
                // search the mac table
                SOCKET dest_sock = get_receivers_socket(table, table_size, mac_dest);

                if(dest_sock != 0){
                    // found in the mac table, send the frame to the corresponding socket
                    printf("found dest_mac in mac table, sending the frame to the corresponding socket\n");
                    send_ethernet_frame(dest_sock, frame);
                }else{
                    // not found in the mac table, broadcast to all
                    printf("dest mac not found in table. broadcasting.\n");
                    for(int i = 0; i<client_size; i++){
                        if(clients[i].s != data.s){
                            send_ethernet_frame(clients[i].s, frame);
                            printf("sent ethernet frame to client %d\n\n", i);
                        }
                    }
                }
            }

        }
    }

    free(frame); // free up the allocated memoru before exiting from this function.
}

int main(int argc, char *argv[])
{
    bool success = init_networking();
    if (!success)
        return 0;

    SOCKET server_socket = create_socket();
    if (!server_socket)
        return 0;

    success = bind_socket(server_socket, 0, 8888);
    if (!success)
        return 0;

    // Opening command thread
    pthread_t command_thread;
    int arg;
    pthread_create(&command_thread, NULL, command_listener, &arg);

    listen_for_connections(server_socket, 1);

    SOCKET client_socket;

    while (1)
    {
        // Accept and incoming connection
        puts("Waiting for incoming connections...");

        client_socket = accept_connection(server_socket);

        puts("\n\n\nConnection accepted");

        // create sending and receiving threads
        struct thread_data td;
        td.s = client_socket;
        clients[client_size].s = client_socket;
        client_size++;

        // create_socket_thread(thread_to_send, td);
        create_socket_thread(thread_to_recv, td);
    }

    // go to sleep
    sleep_for_ever();

    // finally process cleanup job
    closesocket(server_socket);
    closesocket(client_socket);
    WSACleanup();
    return 0;
}
