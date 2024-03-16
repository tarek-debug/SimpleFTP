/* 
 * Name: Tarek Solamy V_1.0 2/27/2024
 * FTP Client Program (ftpclient.c)
 * This program implements a simple FTP client that connects to an FTP server specified by the user.
 * Usage:
 *  $ ./ftpclient server
 *
 * Upon successful connection to the server, the program displays a prompt ('ftp>') for the user to input commands.
 * Implemented commands include:
 *  - get filename: Retrieve a file from the server.
 *  - mget filenames: Retrieve multiple files from the server. (Extra credit)
 *  - put filename: Upload a local file to the server.
 *  - mput filenames: Upload multiple local files to the server. (Extra credit)
 *  - ls: List files in the current remote directory on the server.
 *  - cd: Change the remote directory on the server.
 *  - lcd: Change the local client directory.
 *  - bye: Terminate the FTP session.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

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


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <server>\n", argv[0]);
        exit(1);
    }
    char buf[BUFSIZE], cdInput[BUFSIZ], bufCopy[BUFSIZE], bufCopy2[BUFSIZE] ;
    struct hostent *host;
	struct sockaddr_in server_address, data_address;
    host = gethostbyname(argv[1]);
    if (host == NULL) {
        perror("client: gethostbyname()");
        exit(1);
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    memcpy(&server_address.sin_addr.s_addr, host->h_addr, host->h_length);
    server_address.sin_port = htons(5342);
    int server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd < 0) {
        perror("client: socket()");
        exit(1);
    }

    if (connect(server_sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("client: connect()");
        exit(1);
    }
    char *words[BUFSIZE];
    int count = 0;
    while(1){
        printf("ftp> ");
        count = 0; // Reset count for each command
        memset(buf, 0, sizeof(buf));
        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            // Handle null input or EOF
            printf("No input or EOF encountered.\n");
            continue;
        } else {

            // Remove the newline character at the end of the input
            buf[strcspn(buf, "\n")] = 0;
            
            // Check if the string is empty after removing newline
            if (strlen(buf) == 0) {
                printf("Empty line entered.\n");
                continue;
            } else {
                memset(bufCopy, 0, sizeof(bufCopy));
                strcpy(bufCopy, buf);
                strcpy(bufCopy2, buf);
                //printf(" %s \n",bufCopy);
                // Tokenize the input and count the words
                char *token = strtok(buf, " \t");

                while (token != NULL) {
                words[count++] = token; // Store the token
                token = strtok(NULL, " \t");
                }
                char *cmd = words[0];
                // Check the count to determine the number of inputs
                if (count == 1) {
                    // for cd, once the user enters the cd panel. They just have to write the name of the directory they wish to go to.
                    if (strcmp(cmd, "cd") == 0) {
                        send(server_sockfd, buf, strlen(buf) + 1, 0); // Send initial cd command
                        printf("Entered remote directory change mode. Type 'exit' to return.\n");
                        while (1) {
                            printf("cd> "); 
                            fgets(buf, sizeof(buf), stdin);
                            buf[strcspn(buf, "\n")] = 0; 

                            // Exit the cd mode on client side
                            if (strcmp(buf, "exit") == 0) {
                                send(server_sockfd, "exit", 5, 0); // Notify server to exit cd mode
                                break; 
                            } else {
                                // Send cd command to server
                                send(server_sockfd, buf, strlen(buf) + 1, 0);
                                // Await confirmation/response
                                memset(buf, 0, sizeof(buf));
                                recv(server_sockfd, buf, sizeof(buf), 0);
                                printf("%s\n", buf); // Print server response
                            }
                        }

                    send(server_sockfd, "exit", 5, 0); // Notify server to exit cd mode
                    memset(buf, 0, sizeof(buf)); // Clear the buffer before the next command.
                
                    recv(server_sockfd, buf, sizeof(buf), 0);
                    printf("%s\n", buf); 

                    }
                    else if (strcmp(cmd, "ls") == 0 || strcmp(cmd, "bye") == 0) {
                        send(server_sockfd, buf, strlen(buf) + 1, 0);
                        if (strcmp(cmd, "bye") == 0) {
                            printf("Exiting the program.\n");
                            break;
                        }
                        memset(buf, 0, sizeof(buf)); // Clear the buffer before receiving new data
                        int bytes_received = recv(server_sockfd, buf, sizeof(buf), 0);
                        if (bytes_received > 0) {
                            printf("%s\n", buf); // Print server response
                        } else {
                            perror("Error receiving response.\n");
                        }
                    }
                    else if (strcmp(cmd, "lcd") == 0) {
                        // for lcd, once the user enters the cd panel. They just have to write the name of the directory they wish to go to.
                        printf("Entered local directory change mode. Type 'exit' to return.\n");
                        while(1) {
                            printf("lcd> "); // Prompt for directory change
                            if (fgets(buf, sizeof(buf), stdin) == NULL) {
                                printf("\nExiting local directory change mode due to input error.\n");
                                break; // Handle fgets failure, such as an EOF or read error
                            }
                            buf[strcspn(buf, "\n")] = 0; // Remove newline character

                            // Exit the lcd mode on client side
                            if (strcmp(buf, "exit") == 0) {
                                printf("Exiting local directory change mode. Returning to main.\n\n");
                                break;
                            } else {
                                // Attempt to change directory
                                if (chdir(buf) == 0) {
                                    printf("Current Local Directory changed to %s\n", buf);
                                } else {
                                    perror("Failed to change local directory"); 
                                }
                            }
                        }
                        memset(buf, 0, sizeof(buf)); // Clear the buffer for the next command
                    }

                } 
                // Inside the while loop where the client handles commands
                else if (count == 2 && ((strcmp(cmd, "get") == 0) || (strcmp(cmd, "put") == 0))) {
                    printf("Preparing to send files\n");
                    char pasvPort[4];
                    int data_sockfd;
                    printf("bufCopy: %s \n", bufCopy);
                    send(server_sockfd, bufCopy, strlen(bufCopy) + 1, 0);
                    memset(pasvPort, 0, sizeof(pasvPort));
                    int recv_len = recv(server_sockfd, pasvPort, BUFSIZE-1, 0);

                    int data_port = 0;
                    if (recv_len > 0) {

                    char portCopy[4];
                    if (strlen(pasvPort) > 4){
                        strncpy(portCopy, &pasvPort[5], 4);
                        portCopy[4] = '\0'; // Null-terminate the result
                        data_port = atoi(portCopy);
                        printf("Port number copy string: %s. \n", portCopy);
                        printf("Port number: %d. \n", data_port);
                    } else {
                        pasvPort[recv_len] = '\0'; // Null-terminate received data
                        data_port = atoi(pasvPort);
                        printf("Port number: %d. \n", data_port);


                    }
                    }

                    while (data_port <= 0) {
                        printf("Error receiving proper port number.\n");
                        printf("Retrying...\n");
                        memset(pasvPort, 0, sizeof(pasvPort));
                        recv(server_sockfd, pasvPort, BUFSIZE-1, 0);
                        data_port = atoi(pasvPort);
                    }
                    printf("Successful data port retrieval: %d \n", data_port);
                    memset(buf, 0, sizeof(buf));
                    if(recv(server_sockfd, buf, BUFSIZE-1, 0) <= 0){
                        perror("Error receiving confirmation.\n");
                        continue;
                    }
                    // Now create a new socket for the data connection
                    data_sockfd = socket(AF_INET, SOCK_STREAM, 0);
                    if (data_sockfd < 0) {
                        perror("client: socket() for data connection");
                        continue;
                    }
                    // Set up data address structure
                    memset(&data_address, 0, sizeof(data_address));
                    data_address.sin_family = AF_INET;
                    data_address.sin_port = htons(data_port);
                    memcpy(&data_address.sin_addr, host->h_addr, host->h_length);

                    // Connect to the data socket on the server
                    if (connect(data_sockfd, (struct sockaddr *)&data_address, sizeof(data_address)) < 0) {
                        perror("client for data connection: connect()");
                        continue;
                    }
                    printf("Current command: %s. \n",bufCopy);
                    
                    char *delimiter = " ";
                    cmd = strtok(bufCopy, delimiter);
                    // Token will point to "WORDS".
                    char *filename = strtok(NULL, delimiter);
                    // Implementation for sending/receiving files
                    if (strcmp(cmd, "get") == 0) {
                        // Wait for the server to send the file
                        //char filename[BUFSIZE];
                        //recv(data_sockfd, filename, BUFSIZE, 0); 
                        if (write_file(data_sockfd, filename) == 0) {
                            printf(" Receiving File: %s.\n", filename);
                            //file_transfer_check(data_sockfd);
                        } else {
                            printf("Failed to receive file %s.\n", filename);
                            //file_transfer_check(data_sockfd);
                        }
                    } else if(strcmp(cmd, "put") == 0) {
                        // Send the file to the server
                        if (send_file(data_sockfd, filename) == 0) {
                            printf("Sending File: %s.\n", filename);
                        } else {
                            printf("Failed to send file %s.\n", filename);
                        }
                    }   
                    //printf("File transferred. \n");
                    close(data_sockfd); // Close data connection socket after transfer

                } 
                else if (count >= 3 && ((strcmp(cmd, "mget") == 0) || (strcmp(cmd, "mput") == 0))){
                    // Tokenize the input and count the words
                    token = strtok(bufCopy2, " \t");
                    int index = 0;
                    while (token != NULL) {
                    words[index++] = token; // Store the token
                    token = strtok(NULL, " \t");
                    }
                    cmd = words[0];
                    int file_iter = 1;
                    int total_files = count;

                    while (file_iter < total_files){
                    printf("Preparing to send files\n");
                    char pasvPort[4]; // Increased buffer size for null termination
                    int data_sockfd;
                    printf("bufCopy: %s \n", bufCopy);
                    send(server_sockfd, bufCopy, strlen(bufCopy) + 1, 0);
 

                    // Initial receive block before entering the while loop for retries
                    memset(pasvPort, 0, strlen(pasvPort)); // Ensure buffer is initially zeroez
                    printf("Port number: %s. \n", pasvPort);
                    int recv_len = recv(server_sockfd, pasvPort, BUFSIZE - 1, 0);
                    printf("Port number string: %s. \n", pasvPort);

                    /*
                    I was getting this error on the second file sent or received:
                    Port number: .
                    Port number string: [EOF]6782.
                    Port number string: 0.
                    
                    */
                    int data_port = 0;
                    if (recv_len > 0) {

                    char portCopy[4];
                    if (strlen(pasvPort) > 4){
                        strncpy(portCopy, &pasvPort[5], 4);
                        portCopy[4] = '\0'; // Null-terminate the result
                        data_port = atoi(portCopy);
                        printf("Port number copy string: %s. \n", portCopy);
                        printf("Port number: %d. \n", data_port);
                    } else {
                        pasvPort[recv_len] = '\0'; // Null-terminate received data
                        data_port = atoi(pasvPort);
                        printf("Port number: %d. \n", data_port);


                    }
                    }



                    int retryCount = 0;
                    const int MAX_RETRY = 5;
                    memset(buf, 0, sizeof(buf));

                    if (data_port <= 0){
                        // Retry loop for port number retrieval
                        while (data_port <= 0 && retryCount < MAX_RETRY) {
                            printf("Error receiving proper port number. Retrying...\n");
                            if (send(server_sockfd, "Port number request", strlen("Port number request") + 1, 0) < 0){
                                perror("Aborting... Please try again later...");
                                retryCount++;
                                break;
                            }
                            recv_len = recv(server_sockfd, pasvPort, BUFSIZE - 1, 0);
                            if (recv_len > 0) {
                                pasvPort[recv_len] = '\0';
                                data_port = atoi(pasvPort);
                                if (data_port > 0) {
                                    printf("Port number: %d\n", data_port);
                                    break; // Successfully received valid port, break out of the loop
                                }
                            }
                            retryCount++;
                        }
                    }

                    if (data_port <= 0) {
                        printf("Failed to retrieve valid data port after %d attempts.\n", MAX_RETRY);
                        perror("Aborting... Please try again later...");
                        break;
                        // Handle failure (e.g., by terminating or retrying the entire operation)
                    } 
                    printf("Successful data port retrieval: %d\n", data_port);
                    send(server_sockfd, "Successful data port retrieval", strlen("Successful data port retrieval") + 1, 0);

                    // Create a new socket for the data connection
                    data_sockfd = socket(AF_INET, SOCK_STREAM, 0);
                    if (data_sockfd < 0) {
                        perror("client: socket() for data connection");
                        continue;
                    }
                    // Set up data address structure
                    memset(&data_address, 0, sizeof(data_address));
                    data_address.sin_family = AF_INET;
                    data_address.sin_port = htons(data_port);
                    memcpy(&data_address.sin_addr, host->h_addr, host->h_length);

                    // Connect to the data socket on the server
                    if (connect(data_sockfd, (struct sockaddr *)&data_address, sizeof(data_address)) < 0) {
                        perror("client for data connection: connect()");
                        continue;
                    }
                    // Implementation for sending/receiving files
                    if (strcmp(cmd, "mget") == 0) {
                        // Wait for the server to send the file
                        //char filename[BUFSIZE];
                        //recv(data_sockfd, filename, BUFSIZE, 0); 
                        if (write_file(data_sockfd, words[file_iter]) == 0) {
                            printf(" Receiving File: %s.\n", words[file_iter]);
                        } else {
                            printf("Failed to receive file %s.\n", words[file_iter]);
                        }
                    } else if(strcmp(cmd, "mput") == 0) {
                        // Send the file to the server
                        if (send_file(data_sockfd, words[file_iter]) == 0) {
                            printf("Sending File: %s.\n", words[file_iter]);
                        } else {
                            printf("Failed to send file %s.\n", words[file_iter]);
                        }
                    }   
                    close(data_sockfd); // Close data connection socket after transfer
                    file_iter++;
                    sleep(1);
                }

                } else {
                    perror("Unkown command.");
                }

            }

        }
    }
    close(server_sockfd);
    return 0;
}




