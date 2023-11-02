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

void *command_listener(void *arg)
{
    char command[100];

    while (true)
    {
        gets(command);

        if (strcmp(command, "exit") == 0) // exiting
            exit(0);
        // Add your code here

        printf("\n\n\n");
    }
}

// return receiver socket based on given mac addres.
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

    while (true)
    {
        int msg_len = receive_ethernet_frame(data.s, frame);
        if (msg_len < 0)
            break; // once send or receive fails, please return;
        if (msg_len > 0)
        {
            // Add your code here

            puts("\n");
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

    // Openning command thread
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
