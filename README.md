# Network File System (NFS)

## Functionality

1. **Create**: Create files and folders.
   - Client Side: Establish connection with NM, prompt user for action, send Create Request, and receive response.
   - NM Side: Receive Create Request, send CREATE request to SS, handle CREATE_BACKUP requests, and send response to client.
   - SS Side: Initialize, check root folder, register paths, handle CREATE requests, and respond to NM.

2. **Read**: Read file content.
3. **Update**: Update file and folder content.
4. **Delete**: Delete files and folders.
5. **List**: List files and folders from a directory.
6. **Info**: Get additional file information.
7. **Clients [50]**: Directory mounting, reading, writing, retrieving file information, creating, deleting files and folders, and copying files/directories between storage servers.
8. **Other Features**: Support for multiple clients.

## SS File Structure

- **SS_Root_0**: Files/Folders
- **SS_Root_1**: Files/Folders

## Format

### Sending a Request
- Include `requests.h` from Common folder.
- Use `send_<request_type>_request` function.

### Receiving a Request
- Include `requests.h` from Common folder.
- Create a Request object.
- Use `receive_request` function.
- Use `request_type` attribute to determine request type and get corresponding content.

### Sending a Response
- Include `responses.h` from Common folder.
- Use `send_response` function.

### Receiving a Response
- Include `responses.h` from Common folder.
- Use `receive_response` function.

### Request and Response Format
- Each request has a header.
- **Header Format**:
  - Request/Response Type: 1 byte (char)
  - Payload Length: 8 bytes (uint64_t)
- After the header, the payload follows.

### Example
- Creating a file `a.txt`:
  - Header: `type='C', length=5`
  - Payload: `"a.txt"`

### Project Description
- For more details, visit: [Project Description](https://karthikv1392.github.io/cs3301_osn/project/)
