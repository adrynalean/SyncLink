CC = gcc
CFLAGS = -g -Wall -std=gnu17

all: nm.out ss.out client.out

clean:
	rm -r **/*.o **/*.out

nm.out: NM/main.o NM/client_handler.o NM/ss_handler.o Common/requests.o Common/responses.o Common/network_utils.o Common/loggers.o NM/request_handlers/create_request_handler.o NM/ss_info.o NM/trie.o Common/path_utils.o NM/request_handlers/delete_request_handler.o NM/request_handlers/read_request_handler.o NM/request_handlers/copy_request_handler.o NM/hash.o
	$(CC) $(CFLAGS) $^ -o $@

NM/main.o: NM/main.c NM/client_handler.h NM/ss_handler.h
	$(CC) $(CFLAGS) -c NM/main.c -o $@

NM/client_handler.o: NM/client_handler.c NM/client_handler.h Common/requests.h Common/responses.h Common/loggers.h Common/network_config.h NM/request_handlers/create_request_handler.h NM/request_handlers/delete_request_handler.h NM/request_handlers/read_request_handler.h
	$(CC) $(CFLAGS) -c NM/client_handler.c -o $@

NM/ss_handler.o: NM/ss_handler.c NM/ss_handler.h Common/requests.h Common/responses.h NM/ss_info.h Common/loggers.h Common/network_config.h
	$(CC) $(CFLAGS) -c NM/ss_handler.c -o $@

NM/request_handlers/create_request_handler.o: NM/request_handlers/create_request_handler.h NM/request_handlers/create_request_handler.c Common/responses.h Common/loggers.h Common/path_utils.h NM/ss_info.h Common/requests.h
	$(CC) $(CFLAGS) -c NM/request_handlers/create_request_handler.c -o $@

NM/request_handlers/delete_request_handler.o: NM/request_handlers/delete_request_handler.h NM/request_handlers/delete_request_handler.c NM/ss_info.h Common/responses.h Common/requests.h
	$(CC) $(CFLAGS) -c NM/request_handlers/delete_request_handler.c -o $@

NM/request_handlers/copy_request_handler.o: NM/request_handlers/copy_request_handler.h NM/request_handlers/copy_request_handler.c NM/ss_info.h Common/responses.h
	$(CC) $(CFLAGS) -c NM/request_handlers/copy_request_handler.c -o $@

NM/request_handlers/read_request_handler.o: NM/request_handlers/read_request_handler.c NM/request_handlers/read_request_handler.h Common/responses.h Common/loggers.h NM/ss_info.h
	$(CC) $(CFLAGS) -c NM/request_handlers/read_request_handler.c -o $@

NM/ss_info.o: NM/ss_info.c NM/ss_info.h NM/trie.h Common/network_config.h NM/hash.h
	$(CC) $(CFLAGS) -c NM/ss_info.c -o $@

NM/trie.o: NM/trie.c NM/trie.h
	$(CC) $(CFLAGS) -c NM/trie.c -o $@

NM/hash.o: NM/hash.c NM/hash.h Common/requests.h
	$(CC) $(CFLAGS) -c NM/hash.c -o $@

Common/requests.o: Common/requests.c Common/requests.h Common/network_utils.h
	$(CC) $(CFLAGS) -c Common/requests.c -o $@

Common/responses.o: Common/responses.c Common/responses.h Common/network_utils.h Common/requests.h
	$(CC) $(CFLAGS) -c Common/responses.c -o $@

Common/network_utils.o: Common/network_utils.c Common/network_utils.h
	$(CC) $(CFLAGS) -c Common/network_utils.c -o $@

Common/loggers.o: Common/loggers.c Common/loggers.h Common/colors.h Common/responses.h
	$(CC) $(CFLAGS) -c Common/loggers.c -o $@

Common/path_utils.o: Common/path_utils.c Common/path_utils.h
	$(CC) $(CFLAGS) -c Common/path_utils.c -o $@

ss.out: SS/main.o SS/initialize.o Common/requests.o Common/responses.o Common/network_utils.o  SS/client_handler.o SS/nm_handler.o Common/loggers.o SS/create.o SS/delete.o SS/file_lock_master_lock.o
	$(CC) $(CFLAGS) $^ -o $@

SS/main.o: SS/main.c SS/initialize.h SS/nm_handler.h SS/client_handler.h  
	$(CC) $(CFLAGS) -c SS/main.c -o $@

SS/nm_handler.o: SS/nm_handler.c SS/nm_handler.h Common/network_config.h Common/loggers.h Common/requests.h Common/responses.h SS/create.h SS/delete.h
	$(CC) $(CFLAGS) -c SS/nm_handler.c -o $@

SS/client_handler.o: SS/client_handler.c SS/client_handler.h Common/loggers.h Common/network_config.h Common/requests.h Common/responses.h SS/create.h SS/delete.h SS/file_lock_master_lock.h
	$(CC) $(CFLAGS) -c SS/client_handler.c -o $@

SS/initialize.o: SS/initialize.c SS/initialize.h Common/network_config.h Common/requests.h Common/responses.h Common/loggers.h
	$(CC) $(CFLAGS) -c SS/initialize.c -o $@

SS/create.o: SS/create.c SS/create.h Common/requests.h
	$(CC) $(CFLAGS) -c SS/create.c -o $@

SS/delete.o: SS/delete.c SS/delete.h Common/requests.h SS/file_lock_master_lock.h
	$(CC) $(CFLAGS) -c SS/delete.c -o $@

SS/file_lock_master_lock.o: SS/file_lock_master_lock.c SS/file_lock_master_lock.h
	$(CC) $(CFLAGS) -c SS/file_lock_master_lock.c -o $@

client.out: Client/main.o Client/read.o Client/write.o Client/create.o Common/responses.o Common/requests.o Common/network_utils.o Common/loggers.o Client/delete.o Client/get.o Client/copy.o Client/list.o	
	$(CC) $(CFLAGS) $^ -o $@

Client/main.o: Client/main.c  Client/create.h Client/write.h Client/delete.h Client/read.h Client/copy.h
	$(CC) $(CFLAGS) -c Client/main.c -o $@

Client/copy.o: Client/copy.c Client/copy.h Common/network_config.h Common/requests.h Common/responses.h Common/loggers.h
	$(CC) $(CFLAGS) -c Client/copy.c -o $@

Client/create.o: Client/create.c Client/create.h Common/network_config.h Common/requests.h Common/responses.h Common/loggers.h
	$(CC) $(CFLAGS) -c Client/create.c -o $@

Client/delete.o: Client/delete.c Client/delete.h Common/network_config.h Common/requests.h Common/responses.h Common/loggers.h
	$(CC) $(CFLAGS) -c Client/delete.c -o $@

Client/read.o: Client/read.c Client/read.h Common/network_config.h Common/requests.h Common/responses.h Common/loggers.h
	$(CC) $(CFLAGS) -c Client/read.c -o $@

Client/write.o: Client/write.c Client/write.h Common/network_config.h Common/requests.h Common/responses.h Common/loggers.h
	$(CC) $(CFLAGS) -c Client/write.c -o $@

Client/get.o: Client/get.c Client/get.h Common/network_config.h Common/requests.h Common/responses.h Common/loggers.h
	$(CC) $(CFLAGS) -c Client/get.c -o $@
#  NOT YET ADDED 
#  write / copy / list / get