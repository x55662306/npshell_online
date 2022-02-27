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

using boost::asio::ip::tcp;
/*
class session
  : public std::enable_shared_from_this<session>
{
private:
  void do_read()
  {
    auto self(shared_from_this());
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        [this, self](boost::system::error_code ec, std::size_t length)
        {
          if (!ec)
          {
            printf("%s", data_);
            do_write();
          }
        });
  }

  void do_write()
  {
    auto self(shared_from_this());
    if((getline(file, line))
    {
      boost::asio::async_write(socket_, boost::asio::buffer(line.c_str(), line.length()),
          [this, self](boost::system::error_code ec, std::size_t)
          {
            if (!ec)
            {
              do_read();
            }
          });
    )
  }

  std::string line;
  std::ifstream file;
  ip::tcp::socket socket_;
  enum { max_length = 1024 };
  char data_[max_length];

public:
  session(ip::tcp::socket socket)
    : socket_(std::move(socket))
  {
  }

  void start()
  {
    file.open("./test_case/t1.txt");
    do_read();
  }


};
*/
class client
{
  

  public:
    
    client(short port) : endpoint(boost::asio::ip::address::from_string("127.0.0.1"), port), socket(my_context)
    {
    	file.open("./test_case/t2.txt", std::ios::in);
    	do_connect();
    }

    void run()
    {
      my_context.run();
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
				printf("connect sucess\n");
				do_read();
			}
     	});
    }

    void do_read()
	{
	    boost::asio::async_read(socket, boost::asio::buffer(data_, max_length), boost::asio::transfer_at_least(2), 
	    [this](boost::system::error_code ec, std::size_t length)
        {
          if (!ec)
          {
          	char tmp_str[max_length] = {};
          	memcpy(tmp_str, data_, length);
          	printf("%s", tmp_str);
          	if(tmp_str[length-2] == '%')
           		do_write();
           	else
           		do_read();
          }
        });
	}

	void do_write()
	{
		memset(line, 0, sizeof(line));
		if(file.getline(line, sizeof(data_)) && !file.eof())
		{
			line[strlen(line)] = '\n';
			printf("%s", line);
			boost::asio::async_write(socket, boost::asio::buffer(line, strlen(line)+1),
		    [this](boost::system::error_code ec, std::size_t /*length*/)
		    {
		      if (!ec)
		      {
		        //do nothing
		      }
		    });
		}
		do_read();
	}

    boost::asio::io_context my_context;
    std::fstream file;
    boost::asio::ip::tcp::endpoint endpoint;
    boost::asio::ip::tcp::socket socket;
    enum { max_length = 102400};
  	char data_[max_length];
  	char line[max_length];

};

void printMainFrame()
{
	std::cout << "Content-type: text/html" << std::endl << std::endl;

	const std::string html_body = R"(<!DOCTYPE html>
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
	        <tr>
	          <th scope="col">nplinux1.cs.nctu.edu.tw:1234</th>
	          <th scope="col">nplinux2.cs.nctu.edu.tw:5678</th>
	        </tr>
	      </thead>
	      <tbody>
	        <tr>
	          <td><pre id="s0" class="mb-0"></pre></td>
	          <td><pre id="s1" class="mb-0"></pre></td>
	        </tr>
	      </tbody>
	    </table>
	  </body>
	</html>)";
	std::cout << html_body << std::endl;

	return 0;
}

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 2)
    {
      std::cerr << "Usage: async_tcp_echo_server <port>\n";
      return 1;
    }

    printMainFrame();

    std::cout << std::getenv("QUERY_STRING") std::endl;

    //client cl(std::atoi(argv[1]));
    //cl.run();
}
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}