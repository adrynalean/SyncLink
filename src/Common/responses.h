#ifndef RESPONSES_H
#define RESPONSES_H

#include <arpa/inet.h>
#include <stdint.h>
#include "requests.h"

// update loggers.c when adding responses
#define NOT_FOUND_RESPONSE '0'
#define INTERNAL_ERROR_RESPONSE '1'
#define INVALID_REQUEST_RESPONSE '2'
#define INVALID_REQUEST_CONTENT_RESPONSE '3'
#define ALREADY_EXISTS_RESPONSE '4'

#define OK_RESPONSE 'a'
#define REDIRECT_RESPONSE 'b'
#define OK_START_STREAM_RESPONSE 'c'

#define MAX_STREAMING_RESPONSE_PAYLOAD_SIZE 1024UL
#define MAX_FILE_SIZE 1024UL * 1024UL // 1MB

int send_response(int socket, char response_type);
int receive_response(int socket, char *response_buffer);

int send_redirect_response_payload(int socket, struct sockaddr_in *address);           // from nm to client
int receive_redirect_response_payload(int socket, struct sockaddr_in *address_buffer); // from client to nm

int send_copied_paths(int socket, uint64_t copied_paths_count, char paths[MAX_ACCESIBLE_PATHS][MAX_PATH_LENGTH]);

int receive_copied_paths(int socket, uint64_t *copied_paths_count_buffer, char path_buffer[MAX_ACCESIBLE_PATHS][MAX_PATH_LENGTH]);

/* Send upto MAX_STREAMING_RESPONSE_PAYLOAD_SIZE */
int send_streaming_response_payload(int socket, char *data, uint64_t size);

int end_streaming_response_payload(int socket);

/* Ensure data_buffer has atleast MAX_STREAMING_RESPONSE_PAYLOAD_SIZE and is memset to 0 */
/* returns size of data read if streamed, 0 if end of stream (no data will be read), -1 if error */
int receive_streaming_response_payload(int socket, char *data_buffer); // until return value < 0 ( read )

#endif // RESPONSES_H