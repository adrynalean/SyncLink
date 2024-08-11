#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>

#include "loggers.h"
#include "colors.h"
#include "responses.h"

void log_info(const char *message, struct sockaddr_in *address)
{
    char datetime[20]; // Buffer for storing the formatted date and time

    // Get the current date and time
    time_t now;
    struct tm *tm_info;
    time(&now);
    tm_info = localtime(&now);
    strftime(datetime, 20, "%Y-%m-%d %H:%M:%S", tm_info);

    // Extract IP address and port information
    char ipAddr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(address->sin_addr), ipAddr, INET_ADDRSTRLEN);
    int port = ntohs(address->sin_port);

    // Log the message in the desired format
    printf(ANSI_COLOR_BLUE "[%s %s:%d] %s\n" ANSI_COLOR_RESET, datetime, ipAddr, port, message);
}

void log_error(const char *error)
{
    fprintf(stderr, ANSI_COLOR_RED "%s" ANSI_COLOR_RESET, error);
}

void log_errno_error(const char *error_format)
{
    const char *error_reason = strerror(errno);
    size_t size = snprintf(NULL, 0, error_format, error_reason);
    char error_message[size + 1];
    sprintf(error_message, error_format, error_reason);
    log_error(error_message);
}

void log_response(char response, struct sockaddr_in *address)
{
    switch (response)
    {
    case OK_RESPONSE:
        log_info("OK_RESPONSE", address);
        break;
    case NOT_FOUND_RESPONSE:
        log_info("NOT_FOUND_RESPONSE", address);
        break;
    case INTERNAL_ERROR_RESPONSE:
        log_info("INTERNAL_ERROR_RESPONSE", address);
        break;
    case INVALID_REQUEST_RESPONSE:
        log_info("INVALID_REQUEST_RESPONSE", address);
        break;
    case INVALID_REQUEST_CONTENT_RESPONSE:
        log_info("INVALID_REQUEST_CONTENT_RESPONSE", address);
        break;
    case ALREADY_EXISTS_RESPONSE:
        log_info("ALREADY_EXISTS_RESPONSE", address);
        break;
    case REDIRECT_RESPONSE:
        log_info("REDIRECT_RESPONSE", address);
        break;
    case OK_START_STREAM_RESPONSE:
        log_info("OK_START_STREAM_RESPONSE", address);
        break;
    default:
        break;
    }
}