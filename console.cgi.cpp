#include <cstdlib>
#include <sstream>
#include <string>
#include <string.h>
#include <iostream>
#include <memory>
#include <fstream>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/system/error_code.hpp>
#include <boost/bind/bind.hpp>
#include <boost/algorithm/string.hpp>
#include <vector>

#define SESS_NUM 5

using boost::asio::ip::tcp;

std::string escape(std::string data)
{
	using boost::algorithm::replace_all;
	data = boost::algorithm::replace_all_copy(data, "&",  "&amp;");
	data = boost::algorithm::replace_all_copy(data, "\"", "&quot;");
	data = boost::algorithm::replace_all_copy(data, "\'", "&apos;");
	data = boost::algorithm::replace_all_copy(data, "<",  "&lt;");
	data = boost::algorithm::replace_all_copy(data, ">",  "&gt;");

	return data;
}

void output_shell(std::string session, std::string content)
{
	content = escape(content);
	content = boost::algorithm::replace_all_copy(content, "\n", "<br />");
	printf("<script>document.getElementById('%s').innerHTML += '%s';</script>", session.c_str(), content.c_str());
    //sys.stdout.flush()

	return;
}

void output_command(std::string session, std::string content)
{
	content = escape(content);
	content = boost::algorithm::replace_all_copy(content, "\n", "<br />");
	printf("<script>document.getElementById('%s').innerHTML += '<b>%s</b>';</script>", session.c_str(), content.c_str());
    //sys.stdout.flush()

	return;
}

std::string convertToString(char* a, int size) 
{ 
	int i; 
	std::string s = ""; 
	for (i = 0; i < size; i++) { 
		s = s + a[i]; 
	} 

	return s; 
}

std::string getNextToken(std::string& s, std::string delimiter)
{
	size_t pos = 0;
	std::string token;

	if(s == "")
		return "";

	pos = s.find(delimiter);
	if(pos == std::string::npos)
	{
		token = s;
		s = "";
	}
	else
	{
		token = s.substr(0, pos);
		s.erase(0, pos + delimiter.length());
	}


	return token;
}


class Session_info
{
public:
	std::string host, file_name;
	int port;

	Session_info(std::string host, int port, std::string file_name)
	{
		this->host = host;
		this->port = port;
		this->file_name = file_name;
	}
};

class client
{


public:

	client(boost::asio::io_context& ioc, std::string s_id, boost::asio::ip::tcp::endpoint end, std::string file_name) :endpoint(end), socket(ioc)
	{
		this->s_id = s_id;
		file.open("./test_case/" + file_name, std::ios::in);
		do_connect();
	}

private:

    //boost::asio::ip::tcp:endpoint m_ep(boost::asio::ip::address::from_string("127.0.0.1"), 1234);

	void do_connect()
	{
		socket.async_connect(endpoint,
			[this](const boost::system::error_code& error)
			{
				if (!error)
				{
  				//printf("connect sucess\n");
					do_read();
				}
				else
				{
					output_command(s_id, "Connect fail");
				}
			});
	}

	void do_read()
	{         
		boost::asio::async_read(socket, boost::asio::buffer(data_, max_length), boost::asio::transfer_at_least(1), 
			[this](boost::system::error_code ec, std::size_t length)
			{
				if (!ec)
				{
					char tmp_str[max_length] = {};
					memcpy(tmp_str, data_, length);
					output_shell(s_id, convertToString(tmp_str, length));
					content += convertToString(tmp_str, length);

					if(content.find("%") != std::string::npos)
					{	
						content = "";
						do_write();
					}
					else
					{
						do_read();
					}
				}
				else
				{
					output_command(s_id, "Read fail");
				}
			});
		return;
	}

	void do_write()
	{
		memset(line, 0, sizeof(line));
		if(file.getline(line, sizeof(data_)) && !file.eof())
		{
			
			if(line[strlen(line)-1] == 13)
				line[strlen(line)-1] = '\n';
			else
				line[strlen(line)] = '\n';
			
			std::string cmd = convertToString(line, strlen(line));
			
			output_command(s_id, cmd);
			boost::asio::async_write(socket, boost::asio::buffer(line, strlen(line)), 
		    [this, cmd](boost::system::error_code ec, std::size_t /*length*/)
			{
				if (!ec)
				{
					if(cmd == "exit\n")
						printRemainingCmd();
					else
						do_read();
				}
				else
				{
					output_command(s_id, "Write fail");
				}
			});
		}
		return;
	}

	void printRemainingCmd()
	{
		while(file.getline(line, sizeof(data_)) && !file.eof())
		{
			if(line[strlen(line)-1] == 13)
				line[strlen(line)-1] = '\n';
			else
				line[strlen(line)] = '\n';

			std::string cmd = convertToString(line, strlen(line));
			output_command(s_id, cmd);
		}

		return;
	}

	boost::asio::ip::tcp::endpoint endpoint;
	std::fstream file;
	boost::asio::ip::tcp::socket socket;
	enum { max_length = 102400};
	char data_[max_length];
	char line[max_length];
	std::string content = "";
	std::string s_id="";
};

void printMainFrame(std::vector<Session_info> session_info_list)
{
	std::cout << "Content-type: text/html" << std::endl << std::endl;

	std::cout << R"(
	<!DOCTYPE html>
	<html lang="en">
	<head>
	<meta charset="UTF-8" />
	<title>NP Project 3 Sample Console</title>
	<link
	rel="stylesheet"
	href="https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/css/bootstrap.min.css"
	integrity="sha384-TX8t27EcRE3e/ihU7zmQxVncDAy5uIKz4rEkgIXeMed4M0jlfIDPvg6uqKI2xXr2"
	crossorigin="anonymous"
	/>
	<link
	href="https://fonts.googleapis.com/css?family=Source+Code+Pro"
	rel="stylesheet"
	/>
	<link
	rel="icon"
	type="image/png"
	href="https://cdn0.iconfinder.com/data/icons/small-n-flat/24/678068-terminal-512.png"
	/>
	<style>
	* {
		font-family: 'Source Code Pro', monospace;
		font-size: 1rem !important;
	}
	body {
		background-color: #212529;
	}
	pre {
		color: #cccccc;
	}
	b {
		color: #01b468;
	}
	</style>
	</head>
	<body>
	<table class="table table-dark table-bordered">
	<thead>
	<tr>)";

	for(int i=0; i<SESS_NUM; i++)
	{
		if(session_info_list[i].host!="")
			std::cout << "<th scope=\"col\">" + session_info_list[i].host + ":" + std::to_string(session_info_list[i].port) + "</th>";
	}

	std::cout << R"( 
	</tr>
	</thead>
	<tbody>
	<tr>)";

	for(int i=0; i<SESS_NUM; i++)
	{
		if(session_info_list[i].file_name!="")
		{  
			std::string id = "s" + std::to_string(i);
			std::cout << "<td><pre id=\""+ id + "\" class=\"mb-0\"></pre></td>";
		}
	}

	std::cout << R"(
	</tr>
	</tbody>
	</table>
	</body>
	</html>)";
	return;
}

void parse_query(std::string query, std::vector<Session_info> *session_info_list)
{
	std::string tmp_host="", tmp_file="", delimiter="";
	int tmp_port;

	for(int i=0; i<6; i++)
	{
		getNextToken(query, "=");
		tmp_host = getNextToken(query, "&");
		getNextToken(query, "=");
		tmp_port = std::atoi(getNextToken(query, "&").c_str());
		getNextToken(query, "=");
		tmp_file = getNextToken(query, "&");
		
		Session_info session_info(tmp_host, tmp_port, tmp_file);
		session_info_list->push_back(session_info);
	}

	return;
}

int main(int argc, char* argv[])
{
	boost::asio::io_context ioc;
	std::vector<Session_info> session_info_list;
	client *client_list[SESS_NUM];
	boost::asio::io_service io_service;
	boost::asio::ip::tcp::resolver resolver(ioc);

	memset(client_list, 0, sizeof(client_list));

	std::string query = std::getenv("QUERY_STRING");

	parse_query(query, &session_info_list);

	printMainFrame(session_info_list);

	
	for(int i=0; i<SESS_NUM; i++)
	{
		if(session_info_list[i].file_name!="")
		{
        //DNS
			boost::asio::ip::tcp::resolver::query addr(session_info_list[i].host, std::to_string(session_info_list[i].port));
			boost::asio::ip::tcp::resolver::iterator iter = resolver.resolve(addr);

			client_list[i] = new client(ioc, "s"+std::to_string(i), iter->endpoint(), session_info_list[i].file_name);
			//cl.run();
		}
	}


	ioc.run();

	return 0;
}