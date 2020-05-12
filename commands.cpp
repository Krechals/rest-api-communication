#include "commands.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "helpers.h"
#include "requests.h"
#include "nlohmann/json.hpp"
#include <string>
#include <iostream>
#define HOST "ec2-3-8-116-10.eu-west-2.compute.amazonaws.com"
#define REGISTER_PATH "/api/v1/tema/auth/register"
#define LOGIN_PATH "/api/v1/tema/auth/login"
#define LIBRARY_PATH "/api/v1/tema/library/access"
#define BOOKS_PATH "/api/v1/tema/library/books"
#define LOGOUT_PATH "/api/v1/tema/auth/logout"
#define DATA_FORMAT "application/json"

void send_registration(int sockfd) {
	std::string user, pass;
	json reg_data;

	// Get username and password form stdin.
	std::cout << "username= ";
    std::cin >> user;
    std::cout << "password= ";
    std::cin >> pass;

	reg_data["username"] = user;
	reg_data["password"] = pass;

	// Send composed message to http server.
	char* message = compute_post_request(HOST, REGISTER_PATH, DATA_FORMAT, 
								   NULL, 0, reg_data.dump().c_str(), NULL);
	send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);

    // Check if server responds with 4xx code.
    if (response[9] == '4') {
    	std::cout << "Username is already taken" << std::endl;
    } else {
    	std::cout << "You have successfuly created an account." << std::endl;
    }
    free(message);
    free(response);
}
std::string send_login(int sockfd, std::string auth_cookie) {
	// Check if we already have an authentification cookie.
	if (auth_cookie == "") {
		json reg_data;
		std::string user, pass;

		// Get username and password from stdin.
		std::cout << "username=";
	    std::cin >> user;
	    std::cout << "password=";
	    std::cin >> pass;
		reg_data["username"] = user;
		reg_data["password"] = pass;

		// Send composed message to http server.
		char* message = compute_post_request(HOST, LOGIN_PATH, DATA_FORMAT, 
								   NULL, 0, reg_data.dump().c_str(), NULL);
		send_to_server(sockfd, message);
	    char *response = receive_from_server(sockfd);

	    // Extract authentification cookie from the response.
	    std::string str_message(response);
		std::string cookie_field = "Set-Cookie: ";
		int index_cookie = str_message.find(cookie_field) + std::string("Set-Cookie: ").size();
		if ((int)str_message.find(cookie_field) == -1) {
			std::cout << "You have entered a wrong username/password." << std::endl;
			auth_cookie = "";
		} else {
			while(str_message[index_cookie] != ';') {
				auth_cookie += str_message[index_cookie];
				index_cookie++;
			}
			std::cout << "You has successfuly logged in." << std::endl;
		}

		free(message);
		free(response);
	    return auth_cookie;
	}
	std::cout << "Already logged in." << std::endl;
	return auth_cookie;
}
std::string enter_library(int sockfd, std::string auth_cookie) {
	char *message, *response;
	std::string token;
	// Check if we already have an authentification cookie.
	if (auth_cookie == "") {
		std::cout << "You are not logged in." << std::endl;
		token = "";
	} else {

		// Send composed message from server.
		char** cookie_used = (char **) malloc(sizeof(char*));
		cookie_used[0] = strdup(auth_cookie.c_str());
		message = compute_get_request(HOST,
								   LIBRARY_PATH, 
								   NULL, cookie_used, 1, NULL);
		send_to_server(sockfd, message);
	    response = receive_from_server(sockfd);

	    // Extract JWT Token from server response.
	    std::string str_message(response);
		std::string token_field = "\"token\":\"";
		int index_token = str_message.find(token_field) + token_field.size();
		while(str_message[index_token] != '\"') {
			token += str_message[index_token];
			index_token++;
		}
		std::cout << "You have successfuly entered the library." << std::endl;
		free(cookie_used[0]);
		free(cookie_used);
		free(message);
		free(response);
	}
	return token;
}
std::vector<json> get_books(int sockfd, std::string auth_cookie, std::string token) {

	// Authentification and Token checking.
	if (auth_cookie.size() == 0) {
		std::cout << "You are not logged in." << std::endl;
		return {};
	}
	if (token.size() == 0) {
		std::cout << "You don't have a library token." << std::endl;
		return {};
	}
	char *message, *response;
	char** cookie_used = (char **) malloc(sizeof(char*));
	cookie_used[0] = strdup(auth_cookie.c_str());
	char* token_used;
	token_used = strdup(token.c_str());

	message = compute_get_request(HOST, BOOKS_PATH, 
								   NULL, cookie_used, 1, token_used);

	send_to_server(sockfd, message);
	response = receive_from_server(sockfd);

	// Extract list of JSONs from server response.
	std::string str_message(response);
	std::string json_field = "[";
	std::string str_json = "";
	int index_json = str_message.find(json_field);
	while(str_message[index_json] != ']') {
		str_json += str_message[index_json];
		index_json++;
	}
	str_json += "]";
	std::vector<json> json_elems = {};
	if (str_json == "[]") {
		std::cout << "You don't have any book in the library." << std::endl;
	} else {
		auto json_list = json::parse(str_json);
		for (auto &json_elem : json_list) {
			json_elems.push_back(json_elem);
		}
	}

	free(token_used);
	free(cookie_used[0]);
	free(cookie_used);
	free(message);
	free(response);
	return json_elems;
}
bool check_id(std::string str) {
	for (auto &chr : str) {
		if (chr < '0' || chr > '9') {
			return false;
		}
	}
	return true;
}
void get_book(int sockfd, std::string auth_cookie, std::string token) {
	// Authentification and Token checking.
	if (auth_cookie.size() == 0) {
		std::cout << "You are not logged in." << std::endl;
		return;
	}
	if (token.size() == 0) {
		std::cout << "You don't have a library token." << std::endl;
		return;
	}
	std::string id;
	std::cin.ignore(256, '\n');
	std::cout << "ID: ";
	std::cin >> id;
	if (!check_id(id)) {
		std::cout << "You did not enter a corrct ID." << std::endl;
		return;
	}

	char *message, *response;
	char** cookie_used = (char **) malloc(sizeof(char*));
	cookie_used[0] = strdup(auth_cookie.c_str());
	char* token_used = strdup(token.c_str());

	std::string url = "/api/v1/tema/library/books/" + id;
	message = compute_get_request(HOST, url.c_str(), 
								   NULL, cookie_used, 1, token_used);

	send_to_server(sockfd, message);
	response = receive_from_server(sockfd);

	// Extract book info from server response.
	std::string str_message(response);
	std::string json_field = "[";
	std::string str_json = "";
	int index_json = str_message.find(json_field);
	if (index_json == -1) {
		// Invalid book ID.
		std::cout << "Invalid book ID." << std::endl;
	} else {
		while(str_message[index_json] != ']') {
			str_json += str_message[index_json];
			index_json++;
		}
		str_json += "]";
		auto json_book = json::parse(str_json);
		std::cout << json_book.dump(4);
	}

	free(cookie_used[0]);
	free(cookie_used);
	free(token_used);
	free(message);
	free(response);
}
void add_book(int sockfd, std::string auth_cookie, std::string token) {
	// Authentification and Token checking.
	if (auth_cookie.size() == 0) {
		std::cout << "You are not logged in." << std::endl;
		return;
	}
	if (token.size() == 0) {
		std::cout << "You don't have a library token." << std::endl;
		return;
	}
	json data_form;
	std::string title, author, genre, publisher;
	int pages;
	std::cin.ignore(256, '\n');

	// Get book info from stdin.
	std::cout << "Title: ";
	getline(std::cin, title);
	if (title.size() > LINELEN) {
		std::cout << "Title name has too many characters." << std::endl;
		return;
	}
	std::cout << "Author: ";
	getline(std::cin, author);
	if (author.size() > LINELEN) {
		std::cout << "Author name has too many characters." << std::endl;
		return;
	}
	std::cout << "Publisher: ";
	getline(std::cin, publisher);
	if (publisher.size() > LINELEN) {
		std::cout << "Publisher name has too many characters." << std::endl;
		return;
	}
	std::cout << "Genre: ";
	getline(std::cin, genre);
	if (genre.size() > LINELEN) {
		std::cout << "Genre name has too many characters." << std::endl;
		return;
	} 
	std::cout << "Pages: ";
	std::cin >> pages;
	if (pages < 0) {
		std::cout << "Page number cannot be negative." << std::endl;
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

	message = compute_post_request(HOST, BOOKS_PATH, DATA_FORMAT, 
								   cookie_used, 1, data_form.dump().c_str(), token_used);

	send_to_server(sockfd, message);
	response = receive_from_server(sockfd);

	// Check if server responses with 4xx code.
	if (response[4] == '9') {
		std::cout << "An error has occured. Try again!" << std::endl;
	} else {
		std::cout << "You have successfuly added a book to the library." << std::endl;
	}

	free(cookie_used[0]);
	free(cookie_used);
	free(token_used);
	free(message);
	free(response);
}
void print_books(std::vector<json> books) {
	for (auto &book : books) {
		std::cout << book.dump(4);
		std::cout << std::endl;
	}
}
void delete_book(int sockfd, std::string auth_cookie, std::string token) {
	// Authentification and Token checking.
	if (auth_cookie.size() == 0) {
		std::cout << "You are not logged in." << std::endl;
		return;
	}
	if (token.size() == 0) {
		std::cout << "You don't have a library token." << std::endl;
		return;
	}
	std::string id;
	std::cin.ignore(256, '\n');

	// Get ID of the string.
	std::cout << "ID: ";
	std::cin >> id;

	// Check if the enterned string is a valid number.
	if (!check_id(id)) {
		std::cout << "You did not enter a corrct ID.";
		return;
	}
	char *message, *response;
	char** cookie_used = (char **) malloc(sizeof(char*));
	cookie_used[0] = strdup(auth_cookie.c_str());
	char* token_used;
	token_used = strdup(token.c_str());

	std::string url = "/api/v1/tema/library/books/" + id;
	message = compute_delete_request(HOST, url.c_str(), 
								   NULL, cookie_used, 1, token_used);

	send_to_server(sockfd, message);
	response = receive_from_server(sockfd);

	std::string str_response = std::string(response);
	
	// Check if server responds with 2xx code. 
	if (str_response[9] == '2') {
		std::cout << "Book was successfuly deleted." << std::endl;
	} else {
		std::cout << "Book ID was not found in the library." << std::endl; 
	}

	free(cookie_used[0]);
	free(cookie_used);
	free(token_used);
	free(message);
	free(response);
}
void logout(int sockfd, std::string &auth_cookie, std::string &token) {
	// Authentification checking.
	if (auth_cookie.size() == 0) {
		std::cout << "You are not logged in." << std::endl;
		return;
	}
	char *message, *response;
	char** cookie_used = (char **) malloc(sizeof(char*));
	cookie_used[0] = strdup(auth_cookie.c_str());

	message = compute_get_request(HOST, LOGOUT_PATH, 
								   NULL, cookie_used, 1, NULL);

	send_to_server(sockfd, message);
	response = receive_from_server(sockfd);

	std::cout << "You have successfuly logged out" << std::endl;
	auth_cookie = "";
	token = "";
	free(cookie_used[0]);
	free(cookie_used);
	free(message);
	free(response);
}