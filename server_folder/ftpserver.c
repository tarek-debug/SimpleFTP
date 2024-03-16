/* 
 * Name: Tarek Solamy V_1.0 2/28/2024
 * FTP Server Program (ftpserver.c)
 *
 * This program implements a simple FTP server that listens for connections from an FTP client. 
 *
 * Usage:
 *  $ ./ftpserver
 *
 * Once started, the server waits for a client connection. Upon establishing a control connection, the server
 * can process multiple commands from the client, including:
 *  - Sending and receiving files (get/put/mget/mput commands).
 *  - Listing directory contents.
 *  - Changing directories on the server side.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dirent.h> 
#define BUFSIZE 1024

int send_file(int sockfd, const char *filename) {
    FILE *infile = fopen(filename, "rb");
    if (infile == NULL) {
        perror("Error opening file");
        return -1;
    }
    char data[BUFSIZE];
    int c;
    while ((c = fread(data, 1, BUFSIZE, infile)) > 0) {
        if (send(sockfd, data, c, 0) < 0) {
            perror("Error sending file.");
            fclose(infile);
            return -1;
        }
    }
    if(ferror(infile)) {
        perror("Error reading from file.");
        fclose(infile);
        return -1;
    }
    fclose(infile);
    return 0;
}

int write_file(int sockfd, const char *filename){
    //open file for writing
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL){
    perror("Error opening file");
    return -1;
    }
    char buffer[BUFSIZE];
    int n;
    // Receive file data and write to file
    while ((n = recv(sockfd, buffer, BUFSIZE, 0)) > 0) {

        if (fwrite(buffer, 1, n, fp) != n){
            perror("Error writing file.");
            fclose(fp);
            return -1;
        }
    }
    if (n < 0) {
        perror("Error reading file");
        fclose(fp);
        return -1;
    }
    fclose(fp);
    return 0;
}

int main(void) {
    int server_sockfd, client_sockfd;
    struct sockaddr_in server_address, client_address;
    socklen_t client_len = sizeof(client_address);
    char buffer[BUFSIZE];
    const char* EOF_SIGNAL = "[EOF]";
    int option = 1;

    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd < 0) {
        perror("Error opening socket");
        exit(1);
    }

    if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option)) < 0) {
        perror("Error setting socket options");
        exit(1);
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(5342);

    if (bind(server_sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Error binding socket");
        close(server_sockfd);
        exit(1);
    }

    printf("Server waiting\n");
    if (listen(server_sockfd, 5) < 0) {
        perror("Error listening on socket");
        close(server_sockfd);
        exit(1);
    }

    while (1) {
        client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);
        if (client_sockfd < 0) {
            perror("Error accepting connection");
            close(server_sockfd);
            continue; // Continue to accept next connection
        }
        char *words[BUFSIZE];
        // Serve this client until "bye" command is received
        while (1) {
            int count = 0;
            memset(words, 0, sizeof(words)); // Reset words array
            memset(buffer, 0, sizeof(buffer)); // Clear buffer for new message
            int n = recv(client_sockfd, buffer, sizeof(buffer), 0);
            if (n <= 0) {
                perror("Error reading from socket or connection closed by client");
                break; // Break inner loop to close client socket and wait for a new connection
            }

            printf("Command received: %s\n", buffer);

            // Tokenize the input and count the words
            char *token = strtok(buffer, " \t");
            // Split the input into words using space as delimiter
            while (token != NULL) {
            words[count++] = token; // Store the token
            token = strtok(NULL, " \t");
            }

           char *cmd = words[0];
            if (count == 1){

                if (strcmp(cmd, "ls") == 0) {
                    char lsOutput[4096] = {0}; // Initialize lsOutput buffer
                    DIR *dp = opendir(".");
                    struct dirent *curdirent;
                    if (dp != NULL) {
                        while ((curdirent = readdir(dp)) != NULL) {
                            strcat(lsOutput, curdirent->d_name);
                            strcat(lsOutput, "\n");
                        }
                        closedir(dp);
                        send(client_sockfd, lsOutput, strlen(lsOutput), 0);
                        //printf("output: %s .\n" , lsOutput);
                    } else {
                        perror("Failed to open directory");
                        continue;
                    }
                }
                else if (strcmp(cmd, "cd") == 0) {
                    while (1) {
                        memset(buffer, 0, sizeof(buffer)); 
                        int n = recv(client_sockfd, buffer, sizeof(buffer) - 1, 0);
                        if (n <= 0) {
                            perror("Error reading from socket or connection closed");
                            break; // Exit loop on error or when connection is closed
                        }

                        buffer[n] = '\0'; //  To null-terminate string

                        if (strcmp(buffer, "exit") == 0) {
                            send(client_sockfd, "Exiting directory change mode.\n", 30, 0);
                            break;
                        } else {
                            // Perform directory change
                            if (chdir(buffer) == -1) {
                                perror("Error changing directory");
                                send(client_sockfd, "Error changing directory.\n", 26, 0);
                            } else {
                                send(client_sockfd, "Directory changed successfully.\n", 32, 0);
                            }
                        }
                    }
                }
                else if (strcmp(cmd, "bye") == 0) {
                    send(client_sockfd, "Server shutting down for this client.\n", 37, 0);
                    break;
                }

            } else if(count == 2){
                char *pasvPortStr="6782";
                int pasvPort;
                // After sending the port number to the client
                while (send(client_sockfd, pasvPortStr , strlen(pasvPortStr) + 1, 0) < 0){
                    perror("Error Sending port number.");
                    perror(" Retrying...");

                }

                int data_sockfd, data_client_sockfd;
                struct sockaddr_in data_addr, data_client_address;
                data_sockfd = socket(AF_INET, SOCK_STREAM, 0);
                if (data_sockfd < 0) {
                    perror("Error opening socket");
                    exit(1);
                }

                if (setsockopt(data_sockfd,SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option)) < 0) {
                    perror("Error setting socket options");
                    exit(1);
                }
                memset(&data_addr, 0, sizeof(data_addr));
                data_addr.sin_family = AF_INET;
                data_addr.sin_addr.s_addr = INADDR_ANY;
                data_addr.sin_port = htons(atoi(pasvPortStr));

                if (bind(data_sockfd, (struct sockaddr *)&data_addr, sizeof(data_addr)) < 0) {
                    perror("Error binding data socket");
                    close(data_sockfd);
                    continue;
                }

                printf("Server waiting\n");
                if (listen(data_sockfd, 5) < 0) {
                    perror("Error listening on data socket");
                    close(data_sockfd);
                    continue;
                }


                send(client_sockfd, "Data Server ready for connection", BUFSIZE, 0);
                
                int data_len = sizeof(data_addr);
                data_client_sockfd = accept(data_sockfd, (struct sockaddr *)&data_client_address, &data_len);
                if (data_client_sockfd < 0) {
                    perror("server: accept on data socket");
                    close(data_sockfd);
                    close(data_client_sockfd);
                    continue;
                }

                char *filename = words[1];
                filename[sizeof(words[1]) - 1] = '\0';
                printf("file name: %s \n", filename);
               // Signal end of file transfer and close data connection
                 if (strcmp(cmd, "get") == 0) {
                    // Server sends file to client
                    //send(data_client_sockfd, filename, strlen(filename), 0); // Send filename first   
                    send_file(data_client_sockfd, filename);
                } else if (strcmp(cmd, "put") == 0) {

                    // Server receives file from client
                    write_file(data_client_sockfd, filename);

                }   
                send(client_sockfd, EOF_SIGNAL, strlen(EOF_SIGNAL), 0); // Send EOF to the client using the connected socket
                close(data_client_sockfd); // Close the client data socket
                close(data_sockfd); // close the listening data socket
                printf("Ready for next command\n");
                
            } // Inside the while loop where commands are processed
            else if (count >= 3 && ((strcmp(cmd, "mget") == 0) || (strcmp(cmd, "mput") == 0))) {
                int portnumber= 6000;
                char pasvPortStr[5];
                //int pasvPort;
                for (int i = 1; i < count; ++i) { // Start from 1 because 0 is the command itself
                    // Convert the integer 'num' to a string
                    //sprintf(pasvPortStr, "%d", port_no_original + i);
                    snprintf(pasvPortStr, sizeof(pasvPortStr), "%d", portnumber);
                    char *filename = words[i];
                     int data_sockfd, data_client_sockfd;
                    struct sockaddr_in data_addr, data_client_address;
                    data_sockfd = socket(AF_INET, SOCK_STREAM, 0);
                    if (data_sockfd < 0) {
                        perror("Error opening socket");
                        exit(1);
                    }

                    if (setsockopt(data_sockfd,SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option)) < 0) {
                        perror("Error setting socket options");
                        exit(1);
                    }
                    memset(&data_addr, 0, sizeof(data_addr));
                    data_addr.sin_family = AF_INET;
                    data_addr.sin_addr.s_addr = INADDR_ANY;
                    printf("Number right before adding to port: %d .\n" , portnumber);
                    data_addr.sin_port = htons(portnumber);
                    portnumber++;

                    if (bind(data_sockfd, (struct sockaddr *)&data_addr, sizeof(data_addr)) < 0) {
                        perror("Error binding data socket");
                        close(data_sockfd);
                        continue;
                    }

                    printf(" Port number to send: %s. \n", pasvPortStr);
                    // Initial send to the client for each file
                    if (send(client_sockfd, pasvPortStr, strlen(pasvPortStr) + 1, 0) < 0){
                        perror(" Failed to send port number. Aborting...\n");
                        close(data_sockfd);
                        continue;
                    }


                    char request[BUFSIZE];
                    memset(request, 0, BUFSIZE); 
                    recv(client_sockfd, request, sizeof(request) - 1, 0); // Receive the first request from the client
                    int retryCount = 0;

                    memset(request, 0, BUFSIZE); 

                    while (strcmp(request, "Port number request") == 0 && retryCount < 5) {
                        printf("Client requested port number retry.\n");

                        send(client_sockfd, pasvPortStr, strlen(pasvPortStr), 0); // Resend port number

                        memset(request, 0, BUFSIZE); 
                        recv(client_sockfd, request, sizeof(request) - 1, 0); // Wait for the next request
                    }
                    if (retryCount == 5){
                        perror(" Failed to send proper proper port number. Aborting... \n");
                        close(data_sockfd);
                        break;
                    }
                    printf("Data connection on server waiting\n");
                    if (listen(data_sockfd, 5) < 0) {
                        perror("Error listening on data socket");
                        close(data_sockfd);
                        continue;
                    }
                    
                    int data_len = sizeof(data_addr);
                    data_client_sockfd = accept(data_sockfd, (struct sockaddr *)&data_client_address, &data_len);
                    if (data_client_sockfd < 0) {
                        perror("server: accept on data socket");
                        close(data_sockfd);
                        close(data_client_sockfd);
                        continue;
                    }

                    // Perform the file transfer
                    if (strcmp(cmd, "mget") == 0) {
                        send_file(data_client_sockfd, filename); // Send file to client
                    } else if (strcmp(cmd, "mput") == 0) {
                        write_file(data_client_sockfd, filename); // Receive file from client
                    }

                    // Close the data connection for the current file and prepare for the next file transfer
                    close(data_client_sockfd);
                    send(client_sockfd, EOF_SIGNAL, strlen(EOF_SIGNAL), 0); // Signal the client that file transfer is complete
                    close(data_client_sockfd); // Close the client data socket
                    close(data_sockfd); // close the listening data socket
                    printf("Ready for next command\n");
                    sleep(1);

                }
                printf("All files have been processed.\n");
                continue;
            }

            else {
                printf("Unkown Command");
                continue;

            }           
         
        }
        fflush(stdout);
        close(client_sockfd); // Close client socket after serving
    }

    close(server_sockfd);
    return 0;
}
