
# SimpleFTP: A Basic File Transfer Protocol Client and Server

SimpleFTP is a lightweight implementation of a basic File Transfer Protocol (FTP) client and server, designed to demonstrate the fundamentals of network programming in C. This project allows users to perform file transfers, directory navigation, and view directory contents over a network connection. 

## Features

- **FTP Server**: Waits for connections from an FTP client, processes commands, and handles file transfers and directory navigation.
- **FTP Client**: Connects to the FTP server, allows the user to input commands for file transfers, directory listings, and directory navigation.
- **Commands Supported**:
  - `get filename`: Retrieve a file from the server.
  - `mget filenames`: Retrieve multiple files from the server. (Extra credit)
  - `put filename`: Upload a local file to the server.
  - `mput filenames`: Upload multiple local files to the server. (Extra credit)
  - `ls`: List files in the current remote directory on the server.
  - `cd`: Change the remote directory on the server.
  - `lcd`: Change the local client directory.
  - `bye`: Terminate the FTP session.

## Getting Started

### Prerequisites

- Linux operating system
- GCC compiler

### Compiling

1. **Server Compilation**
   ```bash
   gcc -o ftpserver ftpserver.c
   ```
2. **Client Compilation**
   ```bash
   gcc -o ftpclient ftpclient.c
   ```

### Running the Application

1. **Start the FTP Server**
   ```bash
   ./ftpserver
   ```
   The server will wait for connections on port 5342.

2. **Connect with the FTP Client**
   ```bash
   ./ftpclient <server_ip_address>
   ```
   Replace `<server_ip_address>` with the IP address of the server.

## Usage

Once connected, the client will present a prompt (`ftp>`) where you can type in commands. Here are some example commands:

- **List server directory contents**:
  ```
  ftp> ls
  ```
- **Change server directory**:
  ```
  ftp> cd <directory>
  ```
- **Get a file from the server**:
  ```
  ftp> get <filename>
  ```
- **Put a file onto the server**:
  ```
  ftp> put <filename>
  ```
- **Terminate the session**:
  ```
  ftp> bye
  ```

## Contributing

Contributions to SimpleFTP are welcome. Please feel free to submit issues and pull requests through GitHub.

## License

This project is open source and available under the MIT License.

## Acknowledgments

- Tarek Solamy, for the initial development of SimpleFTP.
