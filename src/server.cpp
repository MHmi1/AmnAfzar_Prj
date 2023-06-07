#include <iostream> // input output lib
#include <string>	// string class and related methods
#include <sys/socket.h> //socket lib
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h> // multi threading lib
#include <errno.h>
#include <signal.h>
#include <cerrno>
#include <cstring>
#include <bits/stdc++.h>
#include <stdio.h>
#include <vector> //container class
#include <utility> //pair calss
#include <time.h> 
#include <iomanip> //manipulator class
#include <sstream>  
#include <cctype> 
#include "encryption.h"
#include "Exception.h"

using namespace std;

extern size_t generateRandomKey();

inline void tokenize(std::string const &str, const char delim, std::vector<std::string> &out) //toknieize string based on delimeter
					 
{
	std::stringstream ss(str);

	std::string s;
	while (std::getline(ss, s, delim))
	{
		out.push_back(s);
	}
}

vector<pair<int, string>> lookup;

// tuning network parameters
//  server address
#define ADDRESS "0.0.0.0"
// port number
#define PORT 8888
// maximum concurrent connections
#define CONCURRENT_CONNECTION 10
// maximum connection requests queued
#define QUEUE_CONNECTION 20
// buffer size 1KB
#define BUFFER_SIZE 1024
// Thread stack size 64KB
#define THREAD_STACK_SIZE 65536
// current connections
int connection = 0;
// connection handler function
void *connection_handler(void *);

inline bool Validate_Ip(string IP) //validate ipv4 ipv6 address format using regex
{

	// validating IPv4
	regex ipv4("(([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])");

	// validating IPv6
	regex ipv6("((([0-9a-fA-F]){1,4})\\:){7}([0-9a-fA-F]){1,4}");

	// Checking if it is a valid IPv4 addresses
	if (regex_match(IP, ipv4) || regex_match(IP, ipv6))
		return true;
	else
		return false;
}

inline string search_host(int client_id) //function to search for host
{
	for (auto &&[id, ip] : lookup)
	{
		if (client_id == id)
		{
			return ip;
		}
	}

	return "null";
}

inline bool insert_host(int client_id, string ip, int con_id) //function to insert validated host
{

	if (search_host(client_id) == "null")
	{
		try
		{
			lookup.push_back(make_pair(client_id, (ip + ":" + to_string(con_id))));

			return true;
		}
		catch (const bad_alloc &e)
		{
			cout << "-> server Memory Allocation has failed we can't accept new request ! " << e.what() << endl; //memory allocation handling (memory leak)
		}
	}
	return false;
}

int main(int argc, char *argv[])
{
	// thread identifier and attributes
	pthread_t thread_id;
	pthread_attr_t attr;

	if (pthread_attr_init(&attr) != 0)
	{
		cout << "[ERROR][THREAD][INIT] " << stderr << (errno) << "\n";
		return -1;
	}


	if (pthread_attr_setstacksize(&attr, THREAD_STACK_SIZE) != 0)
	{
		cout << "[ERROR][THREAD][STACK] " << stderr << (errno) << "\n";
		return -1;
	}

	if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0)
	{
		cout << "[ERROR][THREAD][DETACH] " << stderr << (errno) << "\n";
		return -1;
	}

	int master_socket, conn_id;
	struct sockaddr_in server, client;

	memset(&server, 0, sizeof(server));
	memset(&client, 0, sizeof(client));

	signal(SIGPIPE, SIG_IGN);

	// creating main socket
	if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		cout << "[ERROR] CAN'T CREATE TO SOCKET\n";
		return -1;
	}
	else
	{
		cout << "[NOTE] SOCKET CREATED DONE\n";
	}

	// Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(ADDRESS);
	server.sin_port = htons(PORT);

	socklen_t addrlen = sizeof(struct sockaddr_in);

	// binding address and port
	if (bind(master_socket, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		cout << "[ERROR][BIND] " << stderr << (errno) << "\n";
		return -1;
	}
	else
	{
		cout << "[NOTE] BIND " << ADDRESS << ":" << PORT << "\n";
	}

	// Listen on the socket, with 20 max connection requests queued
	if (listen(master_socket, QUEUE_CONNECTION) == -1)
	{
		cout << "[ERROR][LISTEN] " << stderr << (errno) << "\n";
		return -1;
	}
	else
	{
		cout << "[INFO] WAITING FOR INCOMING CONNECTIONS\n";
	}

	
	while (true)
	{
		// accept new connections
		conn_id = accept(master_socket, (struct sockaddr *)&client, (socklen_t *)&addrlen);

		// connection acception failed handling
		if (conn_id == -1)
		{
			cout << "[WARNING] CAN'T ACCEPT NEW CONNECTION\n";
		}
		else
		{
			//  connection limit reached handling
			if (connection >= CONCURRENT_CONNECTION)
			{
				cout << "[WARNING] CONNECTION LIMITE REACHED\n";

				close(conn_id); // close connection
			}
			else
			{
				cout << "[INFO] NEW CONNECTION ACCEPTED FROM " << inet_ntoa(client.sin_addr) << ":" << ntohs(client.sin_port) << "\n";
				// new thread for new connection
				if (pthread_create(&thread_id, &attr, connection_handler, new int(conn_id)) == -1)
				{
					cout << "[WARNING] CAN'T CREATE NEW THREAD\n";
					close(conn_id);
				}
				else
				{
					cout << "[INFO] NEW THREAD CREATED\n";
					connection++; // increase connection counter
				}
			}
		}
	}
	return 0;
}

// handle connection for each client
void *connection_handler(void *sock_fd)
{

	clock_t start, end;

	// Recording the starting  tick
	start = clock();

	int read_byte = 0;

	// Get socket descriptor
	int conn_id = *(int *)sock_fd;

	// request data
	char buffer[BUFFER_SIZE] = {0};

	string msg1 = "-> You have been added to server before ! \n";
	string msg2 = "-> you have been added to server succesfully\n";

	string response;
	//  read response continue
	while ((read_byte = recv(conn_id, buffer, BUFFER_SIZE, 0)) > 0)
	{

		if (string(buffer).substr(0, 6) == "client")
		{
			if (search_host((stoi(string(buffer).substr(7)))) != "null")
			{

				send(conn_id, encryptAES(msg1, to_string(generateRandomKey())).c_str(), msg1.size(), 0);
			}
			else
			{

				send(conn_id, encryptAES(msg2, to_string(generateRandomKey())).c_str(), msg2.size(), 0);
				insert_host(stoi(string(buffer).substr(7)), to_string(127)+ " :" +to_string(65536) , conn_id);
			}
			
		}
		else
		{
			vector<std::string> hosts;
			pair<string, string> ips;
			tokenize(string(buffer), ' ', hosts);
			if (hosts.size() == 3)
			{
				try
				{

					Validate_Ip(search_host(stoi(hosts.at(2)))) == true ? ips.first = search_host(stoi(hosts.at(2))) : throw("can't toknize ip of source client !"); // src ip
					Validate_Ip(search_host(stoi(hosts.at(0)))) == true ? ips.second = search_host(stoi(hosts.at(0))) : throw("can't toknize ip of dest client !");	 // dest ip
				}
				catch (...)
				{
					cout << "can't toknize  error " << endl;
				}
			}

			vector<string> src_sock, dest_sock;

			tokenize(ips.first, ':', src_sock);
			tokenize(ips.second, ':', dest_sock);

			if (send(stoi(dest_sock.at(2)), encryptAES(hosts.at(1), to_string(generateRandomKey()).c_str()).c_str(), strlen(hosts.at(1).c_str()), 0) > 0)
			{
				cout << "[SEND] " << response << "\n";
			}
			else
			{
				cout << "[WARNING][SEND] " << stderr << (errno) << "\n";
			}
			cout << "[RECEIVED] " << buffer << "\n";

			memset(buffer, 0, BUFFER_SIZE);
		}
	}

	// terminate connection
	close(conn_id);
	cout << "[INFO] CONNECTION CLOSED\n";

	// decrease connection counts
	connection--;

	// thread automatically terminate after exit connection handler
	cout << "[INFO] THREAD TERMINATED" << std::endl;

	delete (int *)sock_fd;

	// Recording the end clock 
	end = clock();

	// Calculating total time of program runtime
	double time_taken = double(end - start) / double(CLOCKS_PER_SEC);

	// print process time
	cout << "[TIME] PROCESS COMPLETE IN " << std::fixed << time_taken << std::setprecision(4);
	cout << " SEC\n";


	cout << "------------------------\n";

	// exiting
	pthread_exit(NULL);
}
