# SyncLink - Network File System (NFS)

![Network and Communication](https://dtlive.s3.ap-south-1.amazonaws.com/15948/DT_G12_Network-_-Communication-Animated-GIF-Icon-Pack.gif)

## Overview

SyncLink is a Network File System (NFS) designed to facilitate seamless file management across multiple clients. The system supports core operations like creating, reading, updating, deleting, and listing files and directories. With support for up to 50 clients, SyncLink enables efficient handling of file operations across distributed storage servers.

## Functionality

1. **Create**: Allows clients to create files and folders.
   - **Client Side**: Connects to the Network Manager (NM), prompts the user for action, sends a Create Request, and processes the response.
   - **NM Side**: Receives the Create Request, forwards it to the Storage Server (SS), manages CREATE_BACKUP requests, and sends a response back to the client.
   - **SS Side**: Initializes the server, checks the root folder, registers paths, handles CREATE requests, and responds to NM.

2. **Read**: Provides clients with the ability to read file content.
   
3. **Update**: Facilitates updating the content of files and folders.

4. **Delete**: Enables the deletion of files and folders.

5. **List**: Lists the files and folders within a specified directory.

6. **Info**: Retrieves additional information about files.

7. **Clients [50]**: Supports up to 50 clients, allowing for directory mounting, reading, writing, file information retrieval, creation, deletion, and copying of files/directories between storage servers.

8. **Other Features**: Full support for multiple clients performing concurrent operations.

## Storage Server File Structure

- **SS_Root_0**: Contains the primary files and folders.
- **SS_Root_1**: Contains additional files and folders, possibly for backup or distribution.

## Request and Response Format

### Sending a Request

1. Include `requests.h` from the Common folder.
2. Use the `send_<request_type>_request` function.

### Receiving a Request

1. Include `requests.h` from the Common folder.
2. Create a Request object.
3. Use the `receive_request` function.
4. Check the `request_type` attribute to determine the request type and access the corresponding content.

### Sending a Response

1. Include `responses.h` from the Common folder.
2. Use the `send_response` function.

### Receiving a Response

1. Include `responses.h` from the Common folder.
2. Use the `receive_response` function.

### Header Format

Each request includes a header, which is structured as follows:

- **Request/Response Type**: 1 byte (char)
- **Payload Length**: 8 bytes (uint64_t)

The payload immediately follows the header.

### Example: Creating a File

To create a file named `a.txt`:

- **Header**: `type='C', length=5`
- **Payload**: `"a.txt"`
