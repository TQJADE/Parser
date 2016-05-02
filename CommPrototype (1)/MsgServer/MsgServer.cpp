/////////////////////////////////////////////////////////////////////////
// MsgServer.cpp - Demonstrates simple one-way HTTP style messaging    //
//                 and file transfer                                   //
//                                                                     //
// Jim Fawcett, CSE687 - Object Oriented Design, Spring 2016           //
// Application: OOD Project #4                                         //
// Platform:    Visual Studio 2015, Dell XPS 8900, Windows 10 pro      //
/////////////////////////////////////////////////////////////////////////
/*
* This package implements a server that receives HTTP style messages and
* files from multiple concurrent clients and simply displays the messages
* and stores files.
*
* It's purpose is to provide a very simple illustration of how to use
* the Socket Package provided for Project #4.
*/
/*
* Required Files:
*   MsgClient.cpp, MsgServer.cpp
*   HttpMessage.h, HttpMessage.cpp
*   Cpp11-BlockingQueue.h
*   Sockets.h, Sockets.cpp
*   FileSystem.h, FileSystem.cpp
*   Logger.h, Logger.cpp 
*   Utilities.h, Utilities.cpp
*/
/*
 * ToDo:
 * - pull the receiving code into a Receiver class
 * - Receiver should own a BlockingQueue, exposed through a
 *   public method:
 *     HttpMessage msg = Receiver.GetMessage()
 * - You will start the Receiver instance like this:
 *     Receiver rcvr("localhost:8080");
 *     ClientHandler ch;
 *     rcvr.start(ch);
 */
#include "../HttpMessage/HttpMessage.h"
#include "../Sockets/Sockets.h"
#include "../FileSystem/FileSystem.h"
#include "../Logger/Cpp11-BlockingQueue.h"
#include "../Logger/Logger.h"
#include "../Utilities/Utilities.h"
#include "../XmlDocument/XmlElement/XmlElement.h"
#include <time.h>
#include <string>
#include <iostream>
#include <direct.h>
#include <string>
#include <vector>
#include "../VSHelp/DataStore/DataStore.h"
#include "../VSHelp/FileMgr/FileMgr.h"
#include <set>
using Show = StaticLogger<1>;
using namespace Utilities;
using EndPoint = std::string;

/////////////////////////////////////////////////////////////////////
// ClientHandler class
/////////////////////////////////////////////////////////////////////
// - instances of this class are passed by reference to a SocketListener
// - when the listener returns from Accept with a socket it creates an
//   instance of this class to manage communication with the client.
// - You need to be careful using data members of this class
//   because each client handler thread gets a reference to this 
//   instance so you may get unwanted sharing.
// - I may change the SocketListener semantics (this summer) to pass
//   instances of this class by value.
// - that would mean that all ClientHandlers would need either copy or
//   move semantics.
//
class ClientHandler
{
public:
  ClientHandler(BlockingQueue<HttpMessage>& msgQ) : msgQ_(msgQ) {}
  void operator()(Socket socket);
 
private:
  bool connectionClosed_;
  HttpMessage readMessage(Socket& socket);
  bool readFile(const std::string& filename, size_t fileSize, Socket& socket);
  void createXml(std::string path, std::string name, std::vector<std::vector<std::string>>store);
  BlockingQueue<HttpMessage>& msgQ_;
  void sendFolderList(HttpMessage& msg, Socket& socket);
  HttpMessage makeMessage(size_t n, const std::string& body, const EndPoint& ep);
  void sendMessage(HttpMessage& msg, Socket& socket);
  bool sendFile(const std::string& fqname, Socket& socket);
  void sendSelectFolder(std::string path, Socket& socket);
};
void ClientHandler::sendSelectFolder(std::string path, Socket& socket) {
	std::vector<std::string> files = FileSystem::Directory::getFiles(path, "*.cpp");
	for (size_t i = 0; i < files.size(); ++i)
	{
		Show::write("\n\n  sending file " + files[i]);
		sendFile(files[i], socket);
	}

	std::vector<std::string> file = FileSystem::Directory::getFiles(path, "*.h");
	for (size_t i = 0; i < file.size(); ++i)
	{
		Show::write("\n\n  sending file " + file[i]);
		sendFile(file[i], socket);
	}
}
bool ClientHandler::sendFile(const std::string& filename, Socket& socket) {

	std::string fqname = "../TestFiles/" + filename;
	FileSystem::FileInfo fi(fqname);
	size_t fileSize = fi.size();
	std::string sizeString = Converter<size_t>::toString(fileSize);
	FileSystem::File file(fqname);
	file.open(FileSystem::File::in, FileSystem::File::binary);
	if (!file.isGood())
		return false;

	HttpMessage msg = makeMessage(1, "", "localhost::8081");
	msg.addAttribute(HttpMessage::Attribute("file", filename));
	msg.addAttribute(HttpMessage::Attribute("content-length", sizeString));
	sendMessage(msg, socket);
	const size_t BlockSize = 2048;
	Socket::byte buffer[BlockSize];
	while (true)
	{
		FileSystem::Block blk = file.getBlock(BlockSize);
		if (blk.size() == 0)
			break;
		for (size_t i = 0; i < blk.size(); ++i)
			buffer[i] = blk[i];
		socket.send(blk.size(), buffer);
		if (!file.isGood())
			break;
	}
	file.close();
	return true;
}

void ClientHandler::sendMessage(HttpMessage& msg, Socket& socket)
{
	std::string msgString = msg.toString();
	socket.send(msgString.size(), (Socket::byte*)msgString.c_str());
}
//----< this defines processing to frame messages >------------------
void ClientHandler::sendFolderList(HttpMessage& msg, Socket& socket) {
	DataStore ds;
	FileMgr fm("../Repository/", ds);
	fm.search();
	msg.clear();
	std::string folder;
	std::set<std::string> myset;
	for (auto fs : ds)
	{
		//Show::write(fs+" \n");
		myset.insert(fs);
	}
	for (auto fs : myset)
	{
		folder = folder + fs + "\n";
		//Show::write("Folder:" + fs);
	}

	HttpMessage m = makeMessage(1,folder, "localhost::8080");
	m.addAttribute(HttpMessage::Attribute("filePath","filePath"));
	sendMessage(m, socket);
}
HttpMessage ClientHandler::readMessage(Socket& socket)
{
  connectionClosed_ = false;
  HttpMessage msg;

  // read message attributes

  while (true)
  {
    std::string attribString = socket.recvString('\n');
    if (attribString.size() > 1)
    {
      HttpMessage::Attribute attrib = HttpMessage::parseAttribute(attribString);
      msg.addAttribute(attrib);
    }
    else
    {
      break;
    }
  }
  // If client is done, connection breaks and recvString returns empty string

  if (msg.attributes().size() == 0)
  {
    connectionClosed_ = true;
    return msg;
  }
  // read body if POST - all messages in this demo are POSTs


  if (msg.attributes()[0].first == "POST")
  {
    //is this a dependency message?
	std::string dependency = msg.findValue("dependency");
	if (dependency != "")
	{
		size_t numBytes = 0;
		size_t pos = msg.findAttribute("content-length");
		std::vector<std::vector<std::string>> store;
		if (pos < msg.attributes().size())
		{
			numBytes = Converter<size_t>::toValue(msg.attributes()[pos].second);
			Socket::byte* buffer = new Socket::byte[numBytes + 1];
			socket.recv(numBytes, buffer);
			buffer[numBytes] = '\0';
			std::string msgBody(buffer);
			msg.addBody(msgBody);
			std::vector<std::string> tokens;
			std::string buf;
			std::stringstream ss(msgBody);
			while (ss>>buf)
			{
				size_t pos = buf.find(':');
				std::string name1 = buf.substr(0,pos);
				std::string name2 = buf.substr(pos+1);
				tokens.push_back(name1);
				tokens.push_back(name2);
				store.push_back(tokens);
				tokens.clear();
			}
			delete[] buffer;
		}
		

		createXml("../Repository/","dependency",store);
	}

    // is this a file message?
    std::string filename = msg.findValue("file");
    if (filename != "")
    {
      size_t contentSize;
      std::string sizeString = msg.findValue("content-length");
      if (sizeString != "")
        contentSize = Converter<size_t>::toValue(sizeString);
      else
        return msg;

      readFile(filename, contentSize, socket);
    }

    if (filename != "")
    {
      // construct message body

      msg.removeAttribute("content-length");
      std::string bodyString = "<file>" + filename + "</file>";
      std::string sizeString = Converter<size_t>::toString(bodyString.size());
      msg.addAttribute(HttpMessage::Attribute("content-length", sizeString));
      msg.addBody(bodyString);
    }
    else if(msg.bodyString().find("getList"))
    {
		  
			sendFolderList(msg,socket);
	}
      // read message body
	else{
      size_t numBytes = 0;
      size_t pos = msg.findAttribute("content-length");
      if (pos < msg.attributes().size())
      {
        numBytes = Converter<size_t>::toValue(msg.attributes()[pos].second);
        Socket::byte* buffer = new Socket::byte[numBytes + 1];
        socket.recv(numBytes, buffer);
        buffer[numBytes] = '\0';
        std::string msgBody(buffer);
        msg.addBody(msgBody);
        delete[] buffer;
      }
    }

	
  }
  return msg;
}
//----< factory for creating messages >------------------------------
/*
* This function only creates one type of message for this demo.
* - To do that the first argument is 1, e.g., index for the type of message to create.
* - The body may be an empty string.
* - EndPoints are strings of the form ip:port, e.g., localhost:8081. This argument
*   expects the receiver EndPoint for the toAddr attribute.
*/
HttpMessage ClientHandler::makeMessage(size_t n, const std::string& body, const EndPoint& ep)
{
	HttpMessage msg;
	HttpMessage::Attribute attrib;
	EndPoint myEndPoint = "localhost:8081";  // ToDo: make this a member of the sender
											 // given to its constructor.
	switch (n)
	{
	case 1:
		msg.clear();
		msg.addAttribute(HttpMessage::attribute("POST", "Message"));
		msg.addAttribute(HttpMessage::Attribute("mode", "oneway"));
		msg.addAttribute(HttpMessage::parseAttribute("toAddr:" + myEndPoint));
		msg.addAttribute(HttpMessage::parseAttribute("fromAddr:" + ep));

		msg.addBody(body);
		if (body.size() > 0)
		{
			attrib = HttpMessage::attribute("content-length", Converter<size_t>::toString(body.size()));
			msg.addAttribute(attrib);
		}
		break;
	default:
		msg.clear();
		msg.addAttribute(HttpMessage::attribute("Error", "unknown message type"));
	}
	return msg;
}


//createXml files
void ClientHandler::createXml(std::string path, std::string name, std::vector<std::vector<std::string>>store) {
	using sPtr = std::shared_ptr <XmlProcessing::AbstractXmlElement >;

	sPtr root = XmlProcessing::makeTaggedElement("meta");
	//root->addChild(makeTextElement("this is a test");

	sPtr child = XmlProcessing::makeTaggedElement("title");
	child->addChild(XmlProcessing::makeTextElement(name));
	root->addChild(child);
	if (store.size())
	{
		for (int i = 0; i < (int)store.size(); i++)
		{
			sPtr child2 = XmlProcessing::makeTaggedElement("dependency");
			root->addChild(child2);
			sPtr name1 = XmlProcessing::makeTaggedElement("name1");
			name1->addChild(XmlProcessing::makeTextElement(store[i][0]));
			sPtr name2 = XmlProcessing::makeTaggedElement("name2");
			name2->addChild(XmlProcessing::makeTextElement(store[i][1]));
			child2->addChild(name1);
			child2->addChild(name2);
		}
		sPtr status = XmlProcessing::makeTaggedElement("status");
		status->addChild(XmlProcessing::makeTextElement("1"));
		sPtr docEl = makeDocElement(root);
		std::ofstream myfile;
		std::string dir = path + name + ".xml";
		myfile.open(dir);
		myfile << "<?xml version=\"1.0\" encoding=\"utf - 8\"?> ";
		myfile << docEl->toString();
		myfile.close();
	}
}

//----< read a binary file from socket and save >--------------------
/*
 * This function expects the sender to have already send a file message, 
 * and when this function is running, continuosly send bytes until
 * fileSize bytes have been sent.
 */
bool ClientHandler::readFile(const std::string& filename, size_t fileSize, Socket& socket)
{
 
  time_t directorytime;
  time(&directorytime);
  struct tm  timeinfo;
  localtime_s(&timeinfo,&directorytime);
  char str[30];
  asctime_s(str, sizeof str, &timeinfo);
  
  std::string ftime(str);
  ftime.erase(std::remove(ftime.begin(), ftime.end(), ' '), ftime.end()); 
  ftime.erase(std::remove(ftime.begin(), ftime.end(), ':'),ftime.end());
  ftime = ftime.substr(0, ftime.size() - 1);
  //ftime = "test";
  ftime = "../Repository/"+ftime + "/";

  std::string fqname = "../Repository/"+ftime+"/"+filename;

 // _mkdir("..\\Repository\\" );
   _mkdir(ftime.c_str());
  FileSystem::File file(fqname);
  file.open(FileSystem::File::out, FileSystem::File::binary);
  if (!file.isGood())
  {
    /*
     * This error handling is incomplete.  The client will continue
     * to send bytes, but if the file can't be opened, then the server
     * doesn't gracefully collect and dump them as it should.  That's
     * an exercise left for students.
     */
    Show::write("\n\n  can't open file " + fqname);
    return false;
  }

  const size_t BlockSize = 2048;
  Socket::byte buffer[BlockSize];

  size_t bytesToRead;
  while (true)
  {
    if (fileSize > BlockSize)
      bytesToRead = BlockSize;
    else
      bytesToRead = fileSize;

    socket.recv(bytesToRead, buffer);

    FileSystem::Block blk;
    for (size_t i = 0; i < bytesToRead; ++i)
      blk.push_back(buffer[i]);

    file.putBlock(blk);
    if (fileSize < BlockSize)
      break;
    fileSize -= BlockSize;
  }
  file.close();

  //test for createXML
/*  std::vector<std::string> dependency;
  dependency.push_back("A.cpp");
  dependency.push_back("A.h");
  std::vector<std::vector<std::string>> store;
  store.push_back(dependency);
  dependency.clear();
  dependency.push_back("A.cpp");
  dependency.push_back("B.h");
  store.push_back(dependency);
  createXml("../Repository/" + ftime,"Test",store);*/
  return true;
}
//----< receiver functionality is defined by this function >---------

void ClientHandler::operator()(Socket socket)
{
  /*
   * There is a potential race condition due to the use of connectionClosed_.
   * If two clients are sending files at the same time they may make changes
   * to this member in ways that are incompatible with one another.  This
   * race is relatively benign in that it simply causes the readMessage to 
   * be called one extra time.
   *
   * The race is easy to fix by changing the socket listener to pass in a 
   * copy of the clienthandler to the clienthandling thread it created.  
   * I've briefly tested this and it seems to work.  However, I did not want
   * to change the socket classes this late in your project cycle so I didn't
   * attempt to fix this.
   */
  while (true)
  {
    HttpMessage msg = readMessage(socket);
    if (connectionClosed_ || msg.bodyString() == "quit")
    {
      Show::write("\n\n  clienthandler thread is terminating");
      break;
    }
    msgQ_.enQ(msg);
  }
}


//----< test stub >--------------------------------------------------

int main()
{
  ::SetConsoleTitle(L"HttpMessage Server - Runs Forever");

  Show::attach(&std::cout);
  Show::start();
  Show::title("\n  HttpMessage Server started");

  BlockingQueue<HttpMessage> msgQ;

  try
  {
    SocketSystem ss;
    SocketListener sl(8080, Socket::IP6);
    ClientHandler cp(msgQ);
    sl.start(cp);
    /*
     * Since this is a server the loop below never terminates.
     * We could easily change that by sending a distinguished 
     * message for shutdown.
     */
    while (true)
    {
      HttpMessage msg = msgQ.deQ();
      Show::write("\n\n  server recvd message contents:\n" + msg.bodyString());
    }
  }
  catch (std::exception& exc)
  {
    Show::write("\n  Exeception caught: ");
    std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
    Show::write(exMsg);
  }
}