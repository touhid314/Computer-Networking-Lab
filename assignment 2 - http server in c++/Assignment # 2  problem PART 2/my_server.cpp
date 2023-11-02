#include "network_helper.cpp"
#include "http_helper.cpp"
#include "api_helper.cpp"
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

SOCKET client_sockets[100];
const char response_404[] = "HTTP/1.1  404  NOT FOUND\nContent-length: 0\n";
const char response_400[] = "HTTP/1.1 400 BAD REQUEST\nContent-length: 0\n";
const char response_204[] = "HTTP/1.1  204  NO CONTENT\nContent-length: 0\n";
const char response_201[] = "HTTP/1.1 201 CONTENT CREATED\n\nContent-length: 0\n";


void delete_first_char(char* str) {
    int i;
    for (i = 0; str[i] != '\0'; i++) {
        str[i] = str[i + 1];
    }
}

int extractId(const char str[]) {
    int id;
    if (sscanf(str, "/api/%d", &id) == 1) {
        return id;
    } else {
        return -1;  // Failed to extract id
    }
}

void stripRedundantSpaces(char* str) {
    int len = strlen(str);
    int new_pos = 0;
    bool prev_space = true; // Initially set to true to handle leading spaces

    for (int current = 0; current < len; current++) {
        if (isspace((unsigned char)str[current])) {
            if (str[current] == '\n') {
                str[new_pos++] = str[current];
                prev_space = true;
            }
            else if (!prev_space) {
                str[new_pos++] = ' ';
                prev_space = true;
            }
        } else {
            str[new_pos++] = str[current];
            prev_space = false;
        }
    }

    str[new_pos] = '\0';
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

        int flag = 1; //flag = 0 means bad request HTTP 400
        int tok_count = 0;
        char *req_type = "";
        char *file_path = "";
        char *http_ver = "";
        int is_api_req = 0;

        char header[1000];
        char body[5000];
        get_http_header_body(req_recv, header, body);


        // PARSING  HEADER ----------------------------
        char *token = header;
        while (token != NULL) {
            token = get_token(token, delim, buf);
            tok_count += 1;
            //processing token 1. should be GET
            if(tok_count == 1){
                req_type = strdup(buf);
            }
            else if(tok_count == 2){
                file_path = strdup(buf);

                char *result = strstr(file_path, "/api");
                if (result != NULL) is_api_req = 1;
                else is_api_req = 0;
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

        //PROCESS REQUEST -----------------------
        //printf("\nreqtype:%s, filepath:%s, ver:%s, is_api_req:%d \n, flag:%d\n", req_type, file_path, http_ver, is_api_req, flag);

        if(flag == 0){
            // BAD REQUEST
            strcpy(response, response_400);
            //printf("%s", response);
        }
        else{
            // REQUEST FORMAT IS OK
            //--------------------GET REQUEST-------------
            if((strcmp(req_type, "GET")==0)){
                //printf("HANDLING GET...");
                if(is_api_req==0){
                    //PARSING GET /filepath
                    //printf("within GET /filepath");
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
                        strcpy(response,response_404);
                    }
                    else{
                        strcpy(response, "HTTP/1.1  200  OK\nContent-length: XX\n\n");
                        int content_len = strlen(file_cont);
                        sprintf(response, "HTTP/1.1  200  OK\nContent-length: %d\n\n",content_len);

                        strcat(response, file_cont);
                    }
                }
                else{
                    //HANDLE GET API
                    //2 possible formats: 1. filepath: /api/<id>
                    //                      2. filepath: /api but body contains id
                    //                      3. invalid format

                    //1. id on header: /api/<id>
                    char user_name[200];
                    char email[200];
                    int id = extractId(file_path);
                    //printf("id: %d\n", id);

                    // id found on header
                    if(id != -1){
                        int suc = get_record(id, user_name, email);
                        if(suc==0){
                            // id not found on database
                            strcpy(response, response_404);
                        }
                        else{
                            // id found on database
                            char res_body[1000];
                            sprintf(res_body, "id=%d&name=%s&email=%s\n", id, user_name, email);

                            int content_len = strlen(res_body);
                            // making response
                            sprintf(response, "HTTP/1.1  200  OK\nContent-length: %d\n\n%s\n",content_len, res_body);
                        }
                    }
                    // 2. id is on the body
                    else{
                        //look for id in request body
                        //printf("looking for id on request body");

                        if (sscanf(body, "id=%d", &id) == 1) {
                            //extracted id from body
                            int suc = get_record(id, user_name, email);
                            if(suc==0){
                                // id not found on database
                                strcpy(response, response_404);
                            }
                            else{
                                // id found on database
                                char res_body[1000];
                                sprintf(res_body, "id=%d&name=%s&email=%s\n", id, user_name, email);

                                int content_len = strlen(res_body);
                                // making response
                                sprintf(response, "HTTP/1.1  200  OK\nContent-length: %d\n\n%s\n",content_len, res_body);
                            }
                        }
                        else {
                            //3. invalid request format
                            strcpy(response, response_400);
                        }
                    }
                }
            }
            //-----------PUT REQUEST---------
            else if(strcmp(req_type, "PUT")==0){
                //HANDLE PUT
                //printf("HANDLING PUT REQUEST\n");

                stripRedundantSpaces(req_recv);

                int id; char user_name[100]; char email[100];
                int result = sscanf(req_recv, "PUT /api HTTP/1.1\n\nid=%d&name=%[^&]&email=%s",&id, user_name, email);
                if(result==3){
                    //req is in correct format
                    //update the db
                    int suc = update_record(id, user_name, email);
                    if(suc){
                        //user exists and record updated
                        strcpy(response, response_204);
                    }
                    else{
                        //user not found
                        strcpy(response, response_404);
                    }
                }
                else{
                    //bad request
                    strcpy(response, response_400);
                }
            }
            //--------------DELETE REQUEST-------------------
            else if(strcmp(req_type, "DELETE")==0){
                //DELETE DELETE
                //printf("HANDLING DELETE\n");
                stripRedundantSpaces(req_recv);

                int id;
                int result = sscanf(req_recv, "DELETE /api HTTP/1.1\n\nid=%d",&id);
                if(result==1){
                    //try to delete
                    int suc = delete_record(id);
                    if(suc==1){
                        //delete success
                        strcpy(response, response_204);
                    }else{
                        //file not found
                        strcpy(response, response_404);
                    }
                }
                else{
                    //bad request
                    strcpy(response, response_400);
                }
            }
            //--------------POST REQUEST---------------------
            else if(strcmp(req_type, "POST")==0){
                //HANDLE POST
                //printf("HANDLING POST");
                stripRedundantSpaces(req_recv);

                char user_name[100]; char email[100];
                int result = sscanf(req_recv, "POST /api HTTP/1.1\n\nname=%[^&]&email=%s",user_name, email);
                if(result==2){
                    //request format okay
                    int suc = create_record(user_name, email);
                    if(suc==1){
                        //successfully created
                        strcpy(response, response_201);
                    }else{
                        //something went wrong
                        //printf("error creating db entry.\n");
                        strcpy(response, response_400);
                    }
                }
                else{
                    //bad request
                    strcpy(response, response_400);
                }

            }else{
                //printf("unrecognized request %s", req_type);
                strcpy(response, response_400);
            }
        }
        //SEND RESPONSE ----------------------
        bool success = send_to_socket(s, response);
        if(!success){
            //printf("Send failed.\n\n");
            return;
        }
}
    else{
        //printf("empty request");
        strcpy(response, response_400);
        bool success = send_to_socket(s, response);
        if(!success){
            printf("Send failed.\n\n");
            return;
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



