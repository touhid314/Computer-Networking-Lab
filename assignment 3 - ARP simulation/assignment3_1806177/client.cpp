#include "helper.cpp"
#include <string.h>
#include <stdio.h>

//-----------------------my functions
int findMacAndGetIP(const char *input, char *ipBuffer) {
    char command[50];
    char ipAddress[50];

    // Use sscanf to parse the input
    int result = sscanf(input, "%s %s", command, ipAddress);

    // Check if sscanf successfully parsed two values
    if (result == 2 && strcmp(command, "find_mac") == 0) {
        strcpy(ipBuffer, ipAddress);
        return 1; // Success
    } else {
        return 0; // Format does not match
    }
}



//----------------------



char my_ip[5];   // eg- 192.178.255.255   (one extra for storing null char at last)
				 // (only used in client part)
char my_mac[7];  // eg- ab:bc:cd:ef:10:12 (one extra for storing null char at last)

struct arp_table_entry
{
    char ip[5];  // eg- 192.168.255.255
    char mac[7]; // eg- ab:bc:cd:ef:10:12
};

arp_table_entry arp_table[255]; // take a big table
int arp_table_size = 0;

// find the index of the arp table entry for the specified ip
int find_index_using_ip(arp_table_entry *arp_table, int table_size, char *ip)
{
    int i = 0;
    for (i = 0; i < table_size; i++)
    {
        if (strcmp(arp_table[i].ip, ip) == 0)
            return i; // find a match so return this ip
    }
    return -1; // for not finding any entry for the specified ip
}








void thread_to_send(thread_data data)
{
    Sleep(100);

    puts("\n\n\n");

    // take a ethernet frame pointer and allocate memory. Allocation memory is important here.
    ethernet_frame *frame = (ethernet_frame *)malloc(sizeof(ethernet_frame));

    // Add your code here

    char string_ip[50];
    char receiver_ip[5];
    char command[100];

    while (true)
    {
        puts("\n\n");
        gets(command);

        if (strcmp(command, "exit") == 0) // exiting
            exit(0);

        // Add your code here
        if (strcmp(command, "arp -a")==0){
            //print the arp table
            if(arp_table_size == 0){
                printf("ARP table is empty\n\n");
            }else{
                printf("ARP table:\n");
                for(int i=0; i<arp_table_size; i++){
                    printf("Entry %d, ip: ", i+1);
                    print_ip(arp_table[i].ip);
                    printf(" MAC:");
                    print_mac(arp_table[i].mac);
                    puts("\n");
                }
            }

            puts("\n");
            continue;
        }

        if(strcmp(command, "arp -d")==0){
            arp_table_size = 0;
            printf("ARP table deleted\n\n");
            continue;
        }


        //---- implementing find_mac <ip_addr>
        int found = findMacAndGetIP(command, string_ip);

        if (found){
            extract_ip(receiver_ip, string_ip);
            //printf("receiver IP Address: "); print_ip(receiver_ip); puts("\n");
        }
        else{
            printf("Format does not match\n\n\n");
            continue;
        }

        // ip address found , now check if this ip is available in the mac table
        int index = find_index_using_ip(arp_table, arp_table_size, receiver_ip);
        if (index == -1){
            // ip not available in the MAC table
            printf("ip not found on mac table. sending arp request\n");
            // send an ethernet frame with the dest. ip address and dest. mac = FFFF, arp payload operation = 0

            //set fields of the frame
            char broad_mac[7];
            generate_broadcast_mac(broad_mac);

            strcpy(frame->mac_destination, broad_mac); //FFFF
            strcpy(frame->mac_source, my_mac);
            frame->payload.operation = htons(0);
            strcpy(frame->payload.sender_hardware_address, my_mac);
            strcpy(frame->payload.sender_protocol_address, my_ip);
            strcpy(frame->payload.target_protocol_address, receiver_ip);


            // now sending the frame
            send_ethernet_frame(data.s,frame);
            printf("arp request sent to server(switch)\n");

        }else{
            // ip available in the MAC table
            printf("ip addr found in MAC table\n");

            //print the found mac
            char found_mac[7];
            int index = find_index_using_ip(arp_table, arp_table_size, receiver_ip);
            if(index >=0){
                strcpy(found_mac, arp_table[index].mac);
                printf("FOUND MAC IN TABLE: ");
                print_mac(found_mac);
                puts("\n\n");
            }else{
                printf("something went wrong\n\n");
            }

        }
    }

    free(frame); // free up the allocated memory before exiting from this function.
}








void thread_to_recv(thread_data data)
{
    // take a ethernet frame pointer and allocate memory. Allocation memory is important here.
    ethernet_frame *frame = (ethernet_frame *)malloc(sizeof(ethernet_frame));
    char mac_source[7];
    char ip_source[5];

    while (true)
    {
        int msg_len = receive_ethernet_frame(data.s, frame);
        if (msg_len < 0){
            printf("failed to receive ethernet frame\n");
            break; // once send or receive fails, please return;
        }
        if (msg_len > 0)
        {
            // Add your code here
            printf("received frame from server\n");
            //---------handling received frame
            // get sender's IP mac and add it to the table
                // gettin ip and mac
            strcpy(mac_source, frame->mac_source);
            strcpy(ip_source, frame->payload.sender_protocol_address);

                //adding to the table
            struct arp_table_entry new_table_entry;
            strcpy(new_table_entry.ip, ip_source);
            strcpy(new_table_entry.mac, mac_source);
            arp_table[arp_table_size] = new_table_entry;
            arp_table_size++;

            printf("sender's ip, mac added to table:");
            print_ip_and_mac(arp_table[arp_table_size-1].ip, arp_table[arp_table_size-1].mac);
            puts("\n");

            // get operation in the arp payload of the frame
            short op = frame->payload.operation;
                if(op == 0){
                    // if op == 0 then this is an arp request, handle the request
                    printf("found arp request, replying to the request\n");

                    // check if dest ip address == this client's ip add
                    if(strcmp(frame->payload.target_protocol_address, my_ip) == 0){
                        // ip add matched so send reply
                        printf("ip address matched!! sending reply\n");
                        //creating ethernet frame to send
                        struct ethernet_frame arp_reply_frame;

                        strcpy(arp_reply_frame.mac_destination, mac_source);
                        strcpy(arp_reply_frame.mac_source, my_mac);
                        arp_reply_frame.payload.operation = htons(1);// 1 for reply
                        strcpy(arp_reply_frame.payload.sender_hardware_address, my_mac);
                        strcpy(arp_reply_frame.payload.sender_protocol_address, my_ip);
                        strcpy(arp_reply_frame.payload.target_hardware_address, mac_source);
                        strcpy(arp_reply_frame.payload.target_protocol_address, ip_source);

                        //send the arp reply frame
                        send_ethernet_frame(data.s, &arp_reply_frame);
                        printf("arp reply frame sent to server(switch)\n");
                    }else{
                        // ip address did not match, ignore the request
                        printf("destination ip address did not match, so ignoring request\n");
                        continue;
                    }


                }else{
                    // if op == 1, this is an arp reply, handle the reply
                    printf("received arp reply. added mac add to table as per reply\n\n");
                    /*
                    // extract sender's mac and ip address
                    char sender_ip[5];
                    char sender_mac[7];
                    strcpy(sender_mac, frame->payload.sender_hardware_address);
                    strcpy(sender_ip, frame->payload.sender_protocol_address);
                    */

                }

            puts("\n\n");
        }
    }

    free(frame); // free up the allocated memory before exiting from this function.
}













int main(int argc, char *argv[])
{
    bool success = init_networking();
    if (!success)
        return 0;

    SOCKET client_socket = create_socket();
    if (!client_socket)
        return 0;

    char server_ip[16];
    strcpy(server_ip, "127.0.0.1");
    success = connect_to_server(client_socket, server_ip, 8888);
    if (!success)
        return 0;

    // create sending and receiving threads
    thread_data data;
    data.s = client_socket;

    // Add your code here
    //generating my_ip and my_macfind_mac 192.168.4.5
    generate_ip(my_ip);
    generate_mac(my_mac);
    print_ip_and_mac(my_ip, my_mac);

    create_socket_thread(thread_to_send, data);
    create_socket_thread(thread_to_recv, data);

    // go to sleep
    sleep_for_ever();

    // finally process cleanup job
    closesocket(client_socket);
    WSACleanup();
    return 0;
}
