#include <cstdlib>
#include <sstream>
#include <string>
#include <iostream>
#include <memory>
#include <utility>
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
  private:
    boost::asio::io_context my_context;
    //tcp::resolver resolver(io_context);
    //tcp::resolver::results_type endpoints = resolver.resolve("127.0.0.1", "1234");
    boost::asio::ip::tcp::socket socket(my_context);
    boost::asio::ip::tcp:endpoint m_ep(boost::asio::ip::address::from_string("127.0.0.1"), 1234);

    void do_connect()
    {
      sock.async_connect(m_ep, 
      [this](boost::system::error_code ec, ip::tcp::socket socket)
      {
        /*
        if (ec)
        return;
        std::make_shared<session>(std::move(socket))->start(); 
        */
        printf("connect sucess");

      });
    }

  public:
    
    client(short port) : m_ep(ip::address::from_string("127.0.0.1"), port)
    {
      do_connect();
    }

    void run()
    {
      my_socket.run();
    }

};


void printMainFrame()
{
  
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

    client cl(std::atoi(argv[1]));
    cl.run();
    
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}