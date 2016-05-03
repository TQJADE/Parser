/////////////////////////////////////////////////////////////////////////
// MsgClient.cpp - Demonstrates simple one-way HTTP messaging          //
// Author: Qi Tan qtan100@syr.edu                                      //
// Source: Jim Fawcett, CSE687 - Object Oriented Design, Spring 2016   //
// Application: OOD Project #4                                         //
// Platform:    Visual Studio 2015, Dell XPS 8900, Windows 10 pro      //
/////////////////////////////////////////////////////////////////////////
/*
 * This package implements a client that sends HTTP style messages and
 * files to a server that simply displays messages and stores files.
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
 * - pull the sending parts into a new Sender class
 * - You should create a Sender like this:
 *     Sender sndr(endPoint);  // sender's EndPoint
 *     sndr.PostMessage(msg);
 *   HttpMessage msg has the sending adddress, e.g., localhost:8080.
 */
#include "../HttpMessage/HttpMessage.h"
#include "../Sockets/Sockets.h"
#include "../FileSystem/FileSystem.h"
#include "../Logger/Logger.h"
#include "../Utilities/Utilities.h"
#include <string>
#include <iostream>
#include <thread>
#include <direct.h>

using Show = StaticLogger<1>;
using namespace Utilities;
using Utils = StringHelper;

/////////////////////////////////////////////////////////////////////
// ClientCounter creates a sequential number for each client
//
class ClientCounter
{
public:
  ClientCounter() { ++clientCount; }
  size_t count() { return clientCount; }
private:
  static size_t clientCount;
};

size_t ClientCounter::clientCount = 0;

/////////////////////////////////////////////////////////////////////
// MsgClient class
// - was created as a class so more than one instance could be 
//   run on child thread
//
class MsgClient
{
public:
  using EndPoint = std::string;
  void execute(const size_t TimeBetweenMessages, const size_t NumMessages);
private:
  HttpMessage makeMessage(size_t n, const std::string& msgBody, const EndPoint& ep);
  void sendMessage(HttpMessage& msg, Socket& socket);
  void sendDependency(Socket& socket);
  bool sendFile(const std::string& fqname, Socket& socket);
  void sendFolder(Socket& socket);
  void sendDownloadWMessage(Socket& socket);
  void sendGetFoloderMessage(Socket& socket);
  void sendDownloadWithout(Socket& socket, std::string path);
  void sendGetFolderList(Socket& socket);
  HttpMessage readMessage(Socket& socket);
  void receiveFile(Socket& socket);
  void downloadFile(const std::string& filename, size_t fileSize, Socket& socket);
 // bool connectionClosed_;
};
void MsgClient::downloadFile(const std::string& fqname, size_t fileSize, Socket& socket) {
	std::string path = "Download/" + fqname;
	FileSystem::File file(path);
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
}
//receriveFile
void MsgClient::receiveFile(Socket& socket) {
	HttpMessage msg;
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

	std::string filename = msg.findValue("file");
	if (filename != "")
	{
		size_t contentSize;
		std::string sizeString = msg.findValue("content-length");
		if (sizeString != "")
			contentSize = Converter<size_t>::toValue(sizeString);
		downloadFile(filename, contentSize, socket);
		Show::write("Client download file"+filename + "\n");
	}
}
void MsgClient::sendFolder(Socket& si) {
	std::vector<std::string> files = FileSystem::Directory::getFiles("../TestFiles", "*.cpp");
	for (size_t i = 0; i < files.size(); ++i)
	{
		Show::write("\n\n  sending file " + files[i]);
		sendFile(files[i], si);
	}
}

void MsgClient::sendDownloadWMessage(Socket & socket)
{
}

void MsgClient::sendGetFoloderMessage(Socket & socket)
{
}

void MsgClient::sendDownloadWithout(Socket & socket,std::string path)
{
	HttpMessage msg=makeMessage(1, path, "toAddr:localhost:8080");;
	//std::string path = "../Repository/SunMay11903202016";
	Show::write("Send request to download without dependency \n");
	
	msg.addAttribute(HttpMessage::Attribute("SelectFolderWO", "SelectFolderWO"));
	sendMessage(msg, socket);
}

void MsgClient::sendGetFolderList(Socket & socket)
{
	HttpMessage msg = makeMessage(1, "getList", "toAddr:localhost:8080");
	sendMessage(msg, socket);
}

HttpMessage	MsgClient::readMessage(Socket& socket) {
	Show::write("Client readMessage \n");
	//connectionClosed_ = false;
	HttpMessage msg;
	//read attribute
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

	if (msg.attributes().size() == 0)
	{
		return msg;
	}

	if (msg.attributes()[0].first == "POST")
	{
		
		std::string filePath = msg.findValue("filePath");
		if (filePath != "")
		{
			Show::write("Test for get FolderList");
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
				Show::write("getFolderList:" + msgBody + "\n");
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
HttpMessage MsgClient::makeMessage(size_t n, const std::string& body, const EndPoint& ep)
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
    msg.addAttribute(HttpMessage::parseAttribute("toAddr:" + ep));
    msg.addAttribute(HttpMessage::parseAttribute("fromAddr:" + myEndPoint));

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


//----< send message using socket >----------------------------------

void MsgClient::sendMessage(HttpMessage& msg, Socket& socket)
{
  std::string msgString = msg.toString();
  socket.send(msgString.size(), (Socket::byte*)msgString.c_str());
}

//-----< send Dependency using socket
void MsgClient::sendDependency(Socket& socket) {
	std::string dependency;
	dependency = "A.cpp:B.cpp Ctest.h:C.cpp";
	HttpMessage msg = makeMessage(1, dependency, "localhost::8080");
	msg.addAttribute(HttpMessage::Attribute("dependency", "dependency"));
	sendMessage(msg, socket);
	Show::write("\n\n  sending dependency " +dependency);
}
//----< send file using socket >-------------------------------------
/*
 * - Sends a message to tell receiver a file is coming.
 * - Then sends a stream of bytes until the entire file
 *   has been sent.
 * - Sends in binary mode which works for either text or binary.
 */
bool MsgClient::sendFile(const std::string& filename, Socket& socket)
{
  // assumes that socket is connected

  std::string fqname = "../TestFiles/" + filename;
  FileSystem::FileInfo fi(fqname);
  size_t fileSize = fi.size();
  std::string sizeString = Converter<size_t>::toString(fileSize);
  FileSystem::File file(fqname);
  file.open(FileSystem::File::in, FileSystem::File::binary);
  if (!file.isGood())
    return false;
  
  HttpMessage msg = makeMessage(1, "", "localhost::8080");
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
//----< this defines the behavior of the client >--------------------

void MsgClient::execute(const size_t TimeBetweenMessages, const size_t NumMessages)
{
  // send NumMessages messages
  
  ClientCounter counter;
  size_t myCount = counter.count();
  std::string myCountString = Utilities::Converter<size_t>::toString(myCount);

  Show::attach(&std::cout);
  Show::start();

  Show::title(
    "Starting HttpMessage client" + myCountString + 
    " on thread " + Utilities::Converter<std::thread::id>::toString(std::this_thread::get_id())
  );
  try
  {
    SocketSystem ss;
    SocketConnecter si;
    while (!si.connect("localhost", 8080))
    {
      Show::write("\n client waiting to connect");
      ::Sleep(100);
    }
    HttpMessage msg;
	//test for upload files
	//sendFolder(si);
	//test for upload dependency
	//sendDependency(si);
	// send message to check out get all the list of files
	//sendGetFolderList(si);
	//msg = readMessage(si);
	//test for get file
	std::string path = "../Repository/SunMay11903202016";
	sendDownloadWithout(si,path);
	receiveFile(si);
	Show::write("\n\n  client sent\n" + msg.toIndentedString());

    msg = makeMessage(1, "quit", "toAddr:localhost:8080");
    sendMessage(msg, si);
    Show::write("\n\n  client" + myCountString + " sent\n" + msg.toIndentedString());
    Show::write("\n");
    Show::write("\n  All done folks");
  }
  catch (std::exception& exc)
  {
    Show::write("\n  Exeception caught: ");
    std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
    Show::write(exMsg);
  }
}
//----< entry point - runs two clients each on its own thread >------

int main()
{
  ::SetConsoleTitle(L"Clients Running on Threads");

  Show::title("Demonstrating two HttpMessage Clients each running on a child thread");

  MsgClient c1;
  std::thread t1(
    [&]() { c1.execute(100, 20); } // 20 messages 100 millisec apart
  );

  MsgClient c2;
  std::thread t2(
    [&]() { c2.execute(120, 20); } // 20 messages 120 millisec apart
  );
  t1.join();
  t2.join();
}