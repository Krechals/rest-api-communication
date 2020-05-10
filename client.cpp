#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "nlohmann/json.hpp"
#include <string>
#include <iostream>
#define SERVER_IP "3.8.116.10"
#define SERVER_PORT 8080

using json = nlohmann::json;
using namespace std;

void send_registration(int sockfd, string user, string pass) {
	json reg_data;

	reg_data["username"] = user;
	reg_data["password"] = pass;

	char *data = strdup("name=Andrei&subject=protocoale");
	char* message = compute_post_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", 
								   "/api/v1/tema/auth/register", 
								   "application/json", 
								   NULL, 0, reg_data.dump().c_str(), NULL);
	printf("%s\n", message);
	send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    printf("%s\n", response);
}
string send_login(int sockfd, string user, string pass, string auth_cookie) {
	json reg_data;
	reg_data["username"] = user;
	reg_data["password"] = pass;

	if (auth_cookie == "") {
		char* message = compute_post_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", 
								   "/api/v1/tema/auth/login", 
								   "application/json", 
								   NULL, 0, reg_data.dump().c_str(), NULL);
		cout << message << endl;
		send_to_server(sockfd, message);
	    char *response = receive_from_server(sockfd);
	    printf("%s\n", response);

	    string str_message(response);
		string cookie_field = "Set-Cookie: ";
		int index_cookie = str_message.find(cookie_field) + string("Set-Cookie: ").size();
		while(str_message[index_cookie] != ';') {
			auth_cookie += str_message[index_cookie];
			index_cookie++;
		}

	    free(message);
	    free(response);
	    return auth_cookie;
	} else {
		char** cookie_used = (char **) malloc(sizeof(char*));
		cookie_used[0] = strdup(auth_cookie.c_str());
		char* message = compute_post_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", 
								   "/api/v1/tema/auth/login", 
								   "application/json", 
								   cookie_used, 1, reg_data.dump().c_str(), NULL);
		send_to_server(sockfd, message);
	    char *response = receive_from_server(sockfd);
	    printf("%s\n", response);

	    free(message);
	    free(response);
	    return auth_cookie;
	}
}
string enter_library(int sockfd, string auth_cookie) {
	char *message, *response;
	string token;
	if (auth_cookie == "") {
		cout << "You are not logged in." << endl;
		return "";
	} else {
		char** cookie_used = (char **) malloc(sizeof(char*));
		cookie_used[0] = strdup(auth_cookie.c_str());
		message = compute_get_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com",
								   "/api/v1/tema/library/access", 
								   NULL, cookie_used, 1, NULL);
		send_to_server(sockfd, message);
	    response = receive_from_server(sockfd);

	    string str_message(response);
		string token_field = "\"token\":\"";
		int index_token = str_message.find(token_field) + token_field.size();
		while(str_message[index_token] != '\"') {
			token += str_message[index_token];
			index_token++;
		}
	    printf("%s\n", response);
	}
	free(message);
	free(response);
	return token;
}
vector<json> get_books(int sockfd, string auth_cookie, string token) {
	if (auth_cookie.size() == 0) {
		cout << "You are not logged in." << endl;
		return {};
	}
	if (token.size() == 0) {
		cout << "You don't have a library token." << endl;
		return {};
	}
	char *message, *response;
	char** cookie_used = (char **) malloc(sizeof(char*));
	cookie_used[0] = strdup(auth_cookie.c_str());
	char* token_used;
	token_used = strdup(token.c_str());

	message = compute_get_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com",
								   "/api/v1/tema/library/books", 
								   NULL, cookie_used, 1, token_used);

	send_to_server(sockfd, message);
	response = receive_from_server(sockfd);

	string str_message(response);
	string json_field = "[";
	string str_json = "";
	int index_json = str_message.find(json_field);
	while(str_message[index_json] != ']') {
		str_json += str_message[index_json];
		index_json++;
	}
	str_json += "]";
	// cout << str_json;
	auto json_list = json::parse(str_json);
	vector<json> json_elems;
	for (auto &json_elem : json_list) {
		json_elems.push_back(json_elem);
		// cout << json_elem.dump();
	}

	free(message);
	free(response);
	return json_elems;
}
bool check_id(string str) {
	for (auto &chr : str) {
		if (chr < '0' || chr > '9') {
			return false;
		}
	}
	return true;
}
void get_book(int sockfd, string auth_cookie, string token) {
	if (auth_cookie.size() == 0) {
		cout << "You are not logged in." << endl;
		return;
	}
	if (token.size() == 0) {
		cout << "You don't have a library token." << endl;
		return;
	}
	char *message, *response;
	char** cookie_used = (char **) malloc(sizeof(char*));
	cookie_used[0] = strdup(auth_cookie.c_str());
	char* token_used;
	token_used = strdup(token.c_str());

	string id;
	cin.ignore(256, '\n'); //discard up to 256 characters or the '\n', whichever comes first 
	cout << "ID: ";
	cin >> id;
	if (!check_id(id)) {
		cout << "You did not enter a corrct ID.";
		return;
	}
	string url = "/api/v1/tema/library/books/" + id;
	message = compute_get_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com",
								   url.c_str(), 
								   NULL, cookie_used, 1, token_used);

	send_to_server(sockfd, message);
	response = receive_from_server(sockfd);

	string str_message(response);
	string json_field = "[";
	string str_json = "";
	int index_json = str_message.find(json_field);
	if (index_json == -1) {
		// Invalid book ID.
		cout << "Invalid book ID." << endl;
		return;
	}
	while(str_message[index_json] != ']') {
		str_json += str_message[index_json];
		index_json++;
	}
	str_json += "]";
	// cout << str_json;
	auto json_book = json::parse(str_json);
	cout << json_book.dump(4);

	free(message);
	free(response);
}
void add_books(int sockfd, string auth_cookie, string token) {
	if (auth_cookie.size() == 0) {
		cout << "You are not logged in." << endl;
		return;
	}
	if (token.size() == 0) {
		cout << "You don't have a library token." << endl;
		return;
	}
	json data_form;
	string title, author, genre, publisher;
	int pages;
	cin.ignore(256, '\n'); //discard up to 256 characters or the '\n', whichever comes first 


	cout << "Title: ";
	getline(cin, title);
	if (title.size() > LINELEN) {
		cout << "Title name has too many characters." << endl;
		return;
	}
	cout << "Author: ";
	getline(cin, author);
	if (author.size() > LINELEN) {
		cout << "Author name has too many characters." << endl;
		return;
	}
	cout << "Publisher: ";
	getline(cin, publisher);
	if (publisher.size() > LINELEN) {
		cout << "Publisher name has too many characters." << endl;
		return;
	}
	cout << "Genre: ";
	getline(cin, genre);
	if (genre.size() > LINELEN) {
		cout << "Genre name has too many characters." << endl;
		return;
	} 
	cout << "Pages: ";
	cin >> pages;
	if (pages < 0) {
		cout << "Page number cannot be negative." << endl;
		return;
	}

	data_form["title"] = title;
	data_form["author"] = author;
	data_form["publisher"] = publisher;
	data_form["genre"] = genre;
	data_form["page_count"] = pages;

	char *message, *response;
	char** cookie_used = (char **) malloc(sizeof(char*));
	cookie_used[0] = strdup(auth_cookie.c_str());
	char* token_used;
	token_used = strdup(token.c_str());

	message = compute_post_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com", 
								   "/api/v1/tema/library/books", 
								   "application/json", 
								   cookie_used, 1, data_form.dump().c_str(), token_used);

	send_to_server(sockfd, message);
	response = receive_from_server(sockfd);
	free(message);
	free(response);
}
void print_books(vector<json> books) {
	for (auto &book : books) {
		cout << book.dump(4);
		cout << endl;
	}
}
void delete_book(int sockfd, string auth_cookie, string token) {
	if (auth_cookie.size() == 0) {
		cout << "You are not logged in." << endl;
		return;
	}
	if (token.size() == 0) {
		cout << "You don't have a library token." << endl;
		return;
	}
	char *message, *response;
	char** cookie_used = (char **) malloc(sizeof(char*));
	cookie_used[0] = strdup(auth_cookie.c_str());
	char* token_used;
	token_used = strdup(token.c_str());

	string id;
	cin.ignore(256, '\n'); //discard up to 256 characters or the '\n', whichever comes first 
	cout << "ID: ";
	cin >> id;
	if (!check_id(id)) {
		cout << "You did not enter a corrct ID.";
		return;
	}
	string url = "/api/v1/tema/library/books/" + id;
	message = compute_delete_request("ec2-3-8-116-10.eu-west-2.compute.amazonaws.com",
								   url.c_str(), 
								   NULL, cookie_used, 1, token_used);

	send_to_server(sockfd, message);
	response = receive_from_server(sockfd);

	string str_response = string(response);
	if (str_response[9] == '2') {
		cout << "Book was successfuly deleted." << endl;
	} else {
		cout << "Book ID was not found in the library." << endl; 
	}

	free(message);
	free(response);
}
int main() {
	// create an array using push_back

	char *message;
    char *response;
    int sockfd;
    struct sockaddr_in serv_addr;
    fd_set read_fds, tmp_fds;

    // Ex 1.1: GET dummy from main server
    string cmd;
    string user, pass;
    string auth_cookie, token;
    vector<json> books;
    while(1) {
    	cin >> cmd;
    	if (cmd == "register") {
    		sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
    		cout << "username=";
    		cin >> user;
    		cout << "password=";
    		cin >> pass;
    		send_registration(sockfd, user, pass);
    		close(sockfd);
    	} else if (cmd == "login") {
    		sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
    		cout << "username=";
    		cin >> user;
    		cout << "password=";
    		cin >> pass;
    		auth_cookie = send_login(sockfd, user, pass, auth_cookie);
    		cout << auth_cookie << endl;
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
    	} else if (cmd == "add_books") {
    		sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
    		add_books(sockfd, auth_cookie, token);
    		close(sockfd);
    	} else if (cmd == "get_book") {
    		sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
    		get_book(sockfd, auth_cookie, token);
    		close(sockfd);
    	} else if (cmd == "delete_book") {
    		sockfd = open_connection(SERVER_IP, SERVER_PORT, AF_INET, SOCK_STREAM, 0);
    		delete_book(sockfd, auth_cookie, token);
    		close(sockfd);
    	}
    }
    return 0;
}