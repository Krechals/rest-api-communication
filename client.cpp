#include <unistd.h>
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "nlohmann/json.hpp"
#include "commands.h"
#include <string>
#include <iostream>
#define SERVER_IP "3.8.116.10"
#define SERVER_PORT 8080

using json = nlohmann::json;

int main() {
    int sockfd;
    std::string cmd;
    std::string auth_cookie, token;
    std::vector<json> books;

    // Command menu
    while(1) {
    	std::cin >> cmd;
    	if (cmd == "register") {
    		sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
    		send_registration(sockfd);
    		close(sockfd);
    	} else if (cmd == "login") {
    		sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
    		auth_cookie = send_login(sockfd, auth_cookie);
    		close(sockfd);
    	} else if (cmd == "enter_library") {
    		sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
    		token = enter_library(sockfd, auth_cookie);
    		close(sockfd);
    	} else if (cmd == "get_books") {
    		sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
    		books = get_books(sockfd, auth_cookie, token);
    		print_books(books);
    		close(sockfd);
    	} else if (cmd == "add_book") {
    		sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
    		add_book(sockfd, auth_cookie, token);
    		close(sockfd);
    	} else if (cmd == "get_book") {
    		sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
    		get_book(sockfd, auth_cookie, token);
    		close(sockfd);
    	} else if (cmd == "delete_book") {
    		sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
    		delete_book(sockfd, auth_cookie, token);
    		close(sockfd);
    	} else if (cmd == "logout") {
    		sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
    		logout(sockfd, auth_cookie, token);
    		close(sockfd);
    	} else if (cmd == "exit") {
    		break;
    	}
    }
    return 0;
}