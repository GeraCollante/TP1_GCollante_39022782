# 	DIRECTORIES
CLIENT_SOURCE	:=cliente
SERVER_SOURCE	:=server
BINARY_DIR		:=bin
INCLUDE  		:=include
DOXY 			:=doxy
INDEX 			:=$(DOXY)/html/index.html

#	BINARY
CLIENT			:=client
SERVER			:=server
FILES			:=fileserv
AUTH			:=auth

# 	COMPILE OPTIONS 
CC				:= gcc
CFLAGS			:= -I$(INCLUDE) -std=gnu11 -Wall -Werror -pedantic -Wextra -Wconversion -O1
#LDFLAGS			:= -lssl -lcrypto
LDLIBS			:= -lssl -lcrypto
DOXYGEN 		:= doxygen
DOXYFILE 		:= doxyfile
BROWSER			:= google-chrome

# 	SOURCES
SRCSCLIENT		:= client.c prompt.c socket_client.c
SRCSSERVER 		:= server.c socket_server.c mq.c
SRCSAUTH		:= auth.c mq.c
SRCSFILES		:= $(addprefix $(SERVER_SOURCE)/, fileserv.c mq.c md5.c)

#	OBJECTS
OBJSCLIENT		:= $(addprefix $(CLIENT_SOURCE)/, $(SRCSCLIENT:.c=.o))
OBJSSERVER		:= $(addprefix $(SERVER_SOURCE)/, $(SRCSSERVER:.c=.o))
OBJSAUTH		:= $(addprefix $(SERVER_SOURCE)/, $(SRCSAUTH:.c=.o))
#OBJSFILES		:= $(SRCSFILES:.c=.o)

all:	clean create

.PHONY: clean
clean:
	-$(RM) $(BINARY_DIR)/$(CLIENT)
	-$(RM) $(BINARY_DIR)/$(SERVER)
	-$(RM) $(BINARY_DIR)/$(FILES)
	-$(RM) $(BINARY_DIR)/$(AUTH)
	-$(RM) $(OBJSCLIENT)
	-$(RM) $(OBJSSERVER)
	-$(RM) $(OBJSAUTH)

create:	$(BINARY_DIR)/$(CLIENT) $(BINARY_DIR)/$(AUTH) $(BINARY_DIR)/$(SERVER) $(BINARY_DIR)/$(FILES)

.PHONY: server
server:
	./$(BINARY_DIR)/$(SERVER)

.PHONY: client
client:
	./$(BINARY_DIR)/$(CLIENT)

doxygen:
	$(DOXYGEN) $(DOXY)/$(DOXYFILE)
	$(BROWSER) $(INDEX)

$(BINARY_DIR)/$(CLIENT): $(OBJSCLIENT)
	@echo "Client objects created..."
	$(LINK.c) $^ -o $@
	@echo "Client binary created..."

$(BINARY_DIR)/$(AUTH): $(OBJSAUTH)
	@echo "Auth objects created..."
	$(LINK.c) $^ -o $@
	@echo "Auth binary created..."

$(BINARY_DIR)/$(FILES): $(SRCSFILES)
	@echo "Files objects created..."
	$(LINK.c) $^ $(LDLIBS) -o $@
	@echo "Files binary created..."

$(BINARY_DIR)/$(SERVER): $(OBJSSERVER)
	@echo "Server objects created..."
	$(LINK.c) $^ -o $@
	@echo "Server binary created..."