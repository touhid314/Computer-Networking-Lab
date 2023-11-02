#include "helper.cpp"

int main(int argc , char *argv[])
{
    bool success = init_networking();
    if(!success) return 0;

    SOCKET server_socket = create_socket();
    if(!server_socket) return 0;

    success = bind_socket(server_socket, 0, 8888);
    if(!success) return 0;

    listen_for_connections(server_socket, 10);


    //Accept and incoming connection
	puts("Waiting for incoming connections...");

    SOCKET client_socket = accept_connection(server_socket);

	puts("Connection accepted");

    closesocket(server_socket);
	return 0;
}



