#ifndef __REQUESTS_H_
#define __REQUESTS_H_

#include "nlohmann/json.hpp"
using json = nlohmann::json;

/* Registration request for creating an accout.
 * 
 * @param1: Socket file descriptor where we send data.
 */
void send_registration(int sockfd);

/* Login request to the http server.
 * 
 * @param1: Socket file descriptor where we send data.
 * @param2: Authentification cookie of a previous login.
 * @return: Authentification cookie when login takes place.
 */
std::string send_login(int sockfd, std::string auth_cookie);

/* Access request for a specific library.
 * 
 * @param1: Socket file descriptor where we send data.
 * @param2: Authentification cookie for our session.
 * @return: JWT Token for a specific library.
 */
std::string enter_library(int sockfd, std::string auth_cookie);

/* Gets books besic info from the server.
 * 
 * @param1: Socket file descriptor where we send data.
 * @param2: Authentification cookie for our session.
 * @param3: JWT Token for our session.
 * @return: List of books from the server (in JSON format).
 */
std::vector<json> get_books(int sockfd, std::string auth_cookie, std::string token);

/* Prints more info for a specific book.
 * 
 * @param1: Socket file descriptor where we send data.
 * @param2: Authentification cookie for our session.
 * @param3: JWT Token for our session.
 */
void get_book(int sockfd, std::string auth_cookie, std::string token);

/* Adds a book to the library.
 * 
 * @param1: Socket file descriptor where we send data.
 * @param2: Authentification cookie for our session.
 * @param3: JWT Token for our session.
 */
void add_book(int sockfd, std::string auth_cookie, std::string token);

/* Prints all books' info from the server.
 * 
 * @param1: List of books.
 */
void print_books(std::vector<json> books);

/* Deletes a specific book from server using DELETE Request.
 * 
 * @param1: Socket file descriptor where we send data.
 * @param2: Authentification cookie for our session.
 * @param3: JWT Token for our session.
 */
void delete_book(int sockfd, std::string auth_cookie, std::string token);

/* Client logouts form the server.
 * 
 * @param1: Socket file descriptor where we send data.
 * @param2: Authentification cookie for our session.
 * @param3: JWT Token for our session.
 */
void logout(int sockfd, std::string &auth_cookie, std::string &token);
#endif

