#include <boost/asio.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <io.h>
#include <vector>
#include <fstream>
#include <boost/algorithm/string.hpp>

#define SESS_NUM 5

using boost::asio::io_service;
using boost::asio::ip::tcp;

boost::asio::io_context global_io_service;

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

int setenv(const char *name, const char *value, int overwrite)
{
  int errcode = 0;
  if(!overwrite) {
    //size_t envsize = 0;
    //errcode = getenv_s(&envsize, NULL, 0, name);
    //if(errcode || envsize) return errcode;
  }

  return _putenv_s(name, value);
}

std::string  getPanel()
{
  std::string result = "Content-type:text/html\r\n\r\n";  

  result += R"(<!DOCTYPE html>
  <html lang="en">
  <head>
  <title>NP Project 3 Panel</title>
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
  href="https://cdn4.iconfinder.com/data/icons/iconsimple-setting-time/512/dashboard-512.png"
  />
  <style>
  * {
    font-family: 'Source Code Pro', monospace;
  }
  </style>
  </head>
  <body class="bg-secondary pt-5">
  <form action="console.cgi" method="GET">
  <table class="table mx-auto bg-light" style="width: inherit">
  <thead class="thead-dark">
  <tr>
  <th scope="col">#</th>
  <th scope="col">Host</th>
  <th scope="col">Port</th>
  <th scope="col">Input File</th>
  </tr>
  </thead>
  <tbody>
  )";
  //6 Session
  for(int i=0; i<SESS_NUM; i++)
  {
    result += "<tr>\n";
    result += "<th scope=\"row\" class=\"align-middle\">Session "+ std::to_string(i+1) + " </th>\n";
    result += R"(<td>
    <div class="input-group">)";
    result += "<select name=\"h\"" + std::to_string(i) +  "class=\"custom-select\">\n";
    result += "<option></option>";
    //12 server options
    for(int k=0; k<12; k++)
    {
      result += "<option value=\"nplinux" + std::to_string(k+1) + ".cs.nctu.edu.tw\">nplinux" + std::to_string(k+1) + "</option>";
    }
    result += R"(</select>
    <div class="input-group-append">
    <span class="input-group-text">.cs.nctu.edu.tw</span>
    </div>
    </div>
    </td>
    <td>)";
    result += "<input name=\"p" + std::to_string(i) + "\" type=\"text\" class=\"form-control\" size=\"5\" />\n";
    result += R"(</td>
    <td>)";
    result += "<select name=\"f" + std::to_string(i) + "\" class=\"custom-select\">";
    result += "<option></option>";
    //10 txt
    for(int k=0; k<10; k++)
    {
      result += "<option value=\"t" + std::to_string(k+1) + ".txt\">t" + std::to_string(k+1) + ".txt</option>";
    }
    result += R"(</select>
    </td>
    </tr>)";
  }

  result += R"(<tr>
  <td colspan="3"></td>
  <td>
  <button type="submit" class="btn btn-info btn-block">Run</button>
  </td>
  </tr>
  </tbody>
  </table>
  </form>
  </body>
  </html>)";

  return result;
}

std::string getConsoleMainFrame(std::vector<Session_info> session_info_list)
{
  std::string result = "Content-type:text/html\r\n\r\n";  

  result += R"(
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
      result += "<th scope=\"col\">" + session_info_list[i].host + ":" + std::to_string(session_info_list[i].port) + "</th>";
  }

  result += R"( 
  </tr>
  </thead>
  <tbody>
  <tr>)";

  for(int i=0; i<SESS_NUM; i++)
  {
    if(session_info_list[i].file_name!="")
    {  
      std::string id = "s" + std::to_string(i);
      result += "<td><pre id=\""+ id + "\" class=\"mb-0\"></pre></td>";
    }
  }

  result += R"(
  </tr>
  </tbody>
  </table>
  </body>
  </html>)";
  
  return result;
}

class printer
{
public: 
  printer(tcp::socket socket) : socket_(std::move(socket)){cnt = SESS_NUM;}

  void output_shell(std::string s_id, std::string content)
  {
    content = escape(content);
    content = boost::algorithm::replace_all_copy(content, "\n", "<br />");
    content += "<script>document.getElementById('" + s_id + "').innerHTML += '" + content + "';</script>";
      boost::asio::async_write(socket_, boost::asio::buffer(content.c_str(), strlen(content.c_str())),[]
            (boost::system::error_code ec, std::size_t /*length*/) 
      {
        //donothing
      });
  }

  void output_command(std::string s_id, std::string content)
  {
    content = escape(content);
    content = boost::algorithm::replace_all_copy(content, "\n", "<br />");
    content += "<script>document.getElementById('"+ s_id +"').innerHTML += '<b>" + content + "</b>';</script>";
      boost::asio::async_write(socket_, boost::asio::buffer(content.c_str(), strlen(content.c_str())),[]
            (boost::system::error_code ec, std::size_t /*length*/) 
      {
        //donothing
      });
  }

  void countDown()
  {
    cnt--;
    return;
  }

  void close()
  {
    cnt++;
    if(cnt == SESS_NUM)
      socket_.close();
    return;
  }

private:
  tcp::socket socket_;
  int cnt;
};

class console
{
public:

  console(printer* p, std::string s_id, boost::asio::ip::tcp::endpoint end, std::string file_name) :endpoint(end), socket(global_io_service)
  {
    printer_ = p;
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
          printer_->output_shell(s_id, convertToString(tmp_str, length));
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
          printer_->output_command(s_id, "Read fail");
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
      
      //sss[sss.length()-1] = '\r';
      //sss += "n";
      //output_command(s_id, std::to_string(sss.length()));
      printer_->output_command(s_id, cmd);
      boost::asio::async_write(socket, boost::asio::buffer(line, strlen(line)), 
        [this, cmd](boost::system::error_code ec, std::size_t )
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
          printer_->output_command(s_id, "Write fail");
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
      printer_->output_command(s_id, cmd);
    }

    printer_->close();
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
  printer* printer_;
};



class Session : public std::enable_shared_from_this<Session> 
{
public:
  Session(tcp::socket socket, tcp::resolver pass_resolver) : socket_(std::move(socket)), resolver(std::move(pass_resolver)) {}

  void start() { do_read(); }
  void printToSocket(std::string str)
  {
    auto self(shared_from_this());
    boost::asio::async_write(socket_, boost::asio::buffer(str.c_str(), strlen(str.c_str())),[this, self]
          (boost::system::error_code ec, std::size_t /*length*/) 
    {
      //donothing
    });

    return;
  }

private:
  void do_read() 
  {
    auto self(shared_from_this());
    socket_.async_read_some(
      boost::asio::buffer(data_, max_length),
      [this, self](boost::system::error_code ec, std::size_t length) 
      {
        //printf("%s", data_);
        sscanf(data_, "%s %s %s %s %s", REQUEST_METHOD, REQUEST_URI, SERVER_PROTOCOL, blackhole, HTTP_HOST);

        if (!ec)
        {
          do_write(length);
        }
        else
        {
          std::cout << "Fail QAQ" << std::endl;
        }
      });
  }

  void do_write(std::size_t length) 
  {
    auto self(shared_from_this());
    boost::asio::async_write(socket_, boost::asio::buffer(status_str, strlen(status_str)),[this, self]
        (boost::system::error_code ec, std::size_t /*length*/) 
    {
      if (!ec) 
      {
        strcpy(SERVER_ADDR, socket_.local_endpoint().address().to_string().c_str());
        sprintf(SERVER_PORT, "%u", socket_.local_endpoint().port());
        strcpy(REMOTE_ADDR, socket_.remote_endpoint().address().to_string().c_str());
        sprintf(REMOTE_PORT, "%u", socket_.remote_endpoint().port());

        bool flag = false;
        int pos = 0;

        for (int i = 0; i < 1000; i++) 
        {
          if (REQUEST_URI[i] == '\0') 
          {
            break;
          } 
          else if (REQUEST_URI[i] == '?') 
          {
            flag = true;
            continue;
          }

          if (flag) 
          {
            QUERY_STRING[pos] = REQUEST_URI[i];
            pos++;
          }
        }

        for (int i = 1; i < 100; i++) 
        {
          if (REQUEST_URI[i] == '\0' || REQUEST_URI[i] == '?') 
            break;
          EXEC_FILE[i + 1] = REQUEST_URI[i];
        }


        setenv("REQUEST_METHOD", REQUEST_METHOD, 1);
        setenv("REQUEST_URI", REQUEST_URI, 1);
        setenv("QUERY_STRING", QUERY_STRING, 1);
        setenv("SERVER_PROTOCOL", SERVER_PROTOCOL, 1);
        setenv("HTTP_HOST", HTTP_HOST, 1);
        setenv("SERVER_ADDR", SERVER_ADDR, 1);
        setenv("SERVER_PORT", SERVER_PORT, 1);
        setenv("REMOTE_ADDR", REMOTE_ADDR, 1);
        setenv("REMOTE_PORT", REMOTE_PORT, 1);
        setenv("EXEC_FILE", EXEC_FILE, 1);

        //global_io_service.notify_fork(io_service::fork_child);
        int sock = socket_.native_handle();
        std::cout << "Socket fd: " << sock << " " << socket_.is_open() << std::endl;

        if(strcmp(EXEC_FILE, "./panel.cgi") == 0)
          sendPanelCgi();
        else if(strcmp(EXEC_FILE, "./console.cgi") == 0)
          sendConsoleCgi();
      }
    });
  }

  void sendPanelCgi()
  {
    auto self(shared_from_this());
    
    std::string panel = getPanel();
    //std::cout << panel << std::endl;

    boost::asio::async_write(socket_, boost::asio::buffer(panel.c_str(), strlen(panel.c_str())),[this, self]
    (boost::system::error_code ec, std::size_t /*length*/)
    {
      if(!ec)
      {
        std::cout << "Sucess" << std::endl;
      }
    });

    return;
  }

  void sendConsoleCgi()
  {
    //memset(console_list, 0, sizeof(console_list));
    std::string query = convertToString(QUERY_STRING, strlen(QUERY_STRING));
    parse_query(query, &console_info_list);
    
    printToSocket(getConsoleMainFrame(console_info_list));

    printer_ = new printer(std::move(socket_));

    for(int i=0; i<SESS_NUM; i++)
    {
      if(console_info_list[i].file_name!="")
      {
        printer_->countDown();
         //DNS
        boost::asio::ip::tcp::resolver::query addr(console_info_list[i].host, std::to_string(console_info_list[i].port));
        boost::asio::ip::tcp::resolver::iterator iter = resolver.resolve(addr);

        console_list[i] = new console(printer_, "s"+std::to_string(i), iter->endpoint(), console_info_list[i].file_name);
        //cl.run();
      }
    }

    return;
  }

  tcp::socket socket_;
  tcp::resolver resolver;
  enum { max_length = 1024 };
  char data_[max_length];
  char status_str[200] = "HTTP/1.1 200 OK\n";
  char REQUEST_METHOD[20];
  char REQUEST_URI[1000];
  char QUERY_STRING[1000];
  char SERVER_PROTOCOL[100];
  char HTTP_HOST[100];
  char SERVER_ADDR[100];
  char SERVER_PORT[10];
  char REMOTE_ADDR[100];
  char REMOTE_PORT[10];
  char EXEC_FILE[100] = "./";
  char blackhole[100];
  std::vector<Session_info> console_info_list;
  console *console_list[SESS_NUM];
  printer *printer_;
};

class Server 
{
public:
  Server(short port) : acceptor_(global_io_service, tcp::endpoint(tcp::v4(), port)), socket_(global_io_service), resolver(global_io_service) 
  {
    do_accept();
  }

private:
  void do_accept() 
  {
    acceptor_.async_accept(socket_, [this](boost::system::error_code ec) 
    {
      if (!ec)
        std::make_shared<Session>(std::move(socket_), std::move(resolver))->start();

      do_accept();
    });
  }

  tcp::acceptor acceptor_;
  tcp::socket socket_;
  tcp::resolver resolver;
};

int main(int argc, char *argv[]) 
{
  try 
  {
    if (argc != 2) 
    {
      std::cerr << "Usage: async_tcp_echo_Server <port>\n";
      return 1;
    }

    Server s(std::atoi(argv[1]));

    global_io_service.run();
  } 
  catch (std::exception &e) 
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}