#include "network_helper.cpp"
#include "http_helper.cpp"

int is_only_null(const char *str) {
    while (*str != '\0') {
        if (*str != '\0') {
            return 0;  // Not only null characters
        }
        str++;
    }
    return 1;  // Only null characters
}

void thread_to_send(struct thread_data td)
{
    SOCKET s = td.s;
    char req[1000];

    // take http req as input from user
    input_http_request(req);
    if(is_only_null(req)==1){
        strcpy(req, " ");
    }


    // send request to server
    bool success = send_to_socket(s, req);
    if(!success)
    {
        printf("failed to send http request to server. \n");
        return; //once send or receive fails, please return;
    }
    else{
        printf("\n\nsent http request to server. waiting for response... \n\n");
    }
    return;
}

void thread_to_recv(struct thread_data td)
{
    SOCKET s = td.s;
    char msg_recv[1000];

    int msg_len = receive_from_socket(s, msg_recv, 1000);
    if( msg_len < 0)
    {
        return; //once send or receive fails, please return;
    }
    if(msg_len>0) printf("RECEIVED HTTP RESPONSE:\n---->\n%s\n<----\n", msg_recv);

	return;
}


int main(int argc , char *argv[])
{
    bool success = init_networking();
    if(!success) return 0;

    SOCKET client_socket;

    while(1){
        client_socket = create_socket();
        if(!client_socket) return 0;

        success = connect_to_server(client_socket, "127.0.0.1", 8888);
        if(!success) return 0;

        //create sending and receiving threads
        struct thread_data td;
        td.s = client_socket;

        thread_to_send(td);
        thread_to_recv(td);

        closesocket(client_socket);
    }
	//go to sleep
    sleep_for_ever();


    //finally process cleanup job
    WSACleanup();
	return 0;
}








