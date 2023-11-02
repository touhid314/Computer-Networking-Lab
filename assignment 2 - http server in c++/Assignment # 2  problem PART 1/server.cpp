#include "network_helper.cpp"
#include "http_helper.cpp"
#include <string.h>

SOCKET client_sockets[100];

void delete_first_char(char* str) {
    int i;
    for (i = 0; str[i] != '\0'; i++) {
        str[i] = str[i + 1];
    }
}

void thread_to_recv(struct thread_data td)
{
    SOCKET s = td.s;
    char req_recv[1000];
    char response[3000];
    strcpy(req_recv,"");

    int req_len = receive_from_socket(s, req_recv, 1000);

    if(req_len>0){
        // break into tokens
        //printf("received req: %s \n", req_recv);

        char delim[] = " ";
        char buf[3000];

        char *token = req_recv;

        int flag = 1; //flag = 0 means bad request HTTP 400
        int tok_count = 0;
        char *req_type = "";
        char *file_path = "";
        char *http_ver = "";

        while (token != NULL) {
            token = get_token(token, delim, buf);
            tok_count += 1;
            //processing token 1. should be GET
            if(tok_count == 1){
                if(strcmp(buf, "GET") != 0 ){
                  flag = 0;
                  break;
                }else{
                    req_type = strdup(buf);
                }
            }
            else if(tok_count == 2){
                file_path = strdup(buf);
            }
            else if(tok_count == 3){
                http_ver = strdup(buf);
                if(strcmp(http_ver, "HTTP/1.1") != 0){
                    flag = 0;
                    break;
                }
            }
            else if(tok_count == 4){
                flag = 0;
                break;
            }
            else{
                flag = 0;
            }
        }

        //process tokens
        //printf("\nreqtype:%s, filepath:%s, ver:%s", req_type, file_path, http_ver);

        if(flag == 0){
            // BAD REQUEST
            strcpy(response, "HTTP/1.1 400 BAD REQUEST\nContent-length: 0\n");
            //printf("%s", response);
        }
        else{
            // REQUEST FORMAT IS OK
            int suc;
            char file_cont[5000];
            if(strcmp(file_path, "/") == 0){
                char file_name[] = "index.html";
                suc = read_html_file(file_name, file_cont);
            }
            else{
                delete_first_char(file_path);
                suc = read_html_file(file_path, file_cont);
            }

            if(suc == 0){
                //file not found
                strcpy(response, "HTTP/1.1 404 NOT FOUND\nContent-length: 0\n");
            }
            else{
                ////////////////////////////
                strcpy(response, "HTTP/1.1  200  OK\nContent-length: XX\n\n");
                strcat(response, file_cont);
            }
        }

        //send proper request
        bool success = send_to_socket(s, response);
        if(!success){
            printf("Send failed.\n\n");
            return;
        }
}
    else{
        printf("empty request. returning... ");
        return;
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

    int client_id=100;
    while(1)
    {

        //Accept and incoming connection
        puts("Waiting for incoming connections...");

        SOCKET s = accept_connection(server_socket);
        client_id++;
        client_sockets[client_id - 100] = s; //store the socket in global variable

        puts("Connection accepted");

        //create sending and receiving threads
        struct thread_data td;
        td.s = s;
        td.client_id = client_id;

        create_socket_thread(thread_to_recv, td);
    }


	//go to sleep
    sleep_for_ever();

    //finally process cleanup job
    closesocket(server_socket);
   //closesocket(s);
    WSACleanup();
	return 0;
}



