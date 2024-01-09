
////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <iostream>
#include <cstdlib>
#include <SFML/Network.hpp>
#include <cstring>
#include <atomic>
#include <thread>
#include <vector>
#include <list>

// Create a socket for communicating with the server
sf::TcpSocket socket;

struct tcpMessage {
    unsigned char nVersion;
    unsigned char nType;
    unsigned short nMsgLen;
    char chMsg[1000];
};

std::atomic<bool> quitSocket(false);
std::vector<tcpMessage> messages;

sf::Packet& operator <<(sf::Packet& Packet, const tcpMessage& C)
{
    return Packet << C.nVersion << C.nType << C.nMsgLen << C.chMsg;
}

sf::Packet& operator >>(sf::Packet& Packet, tcpMessage& C)
{
    return Packet >> C.nVersion >> C.nType >> C.nMsgLen >> C.chMsg;
}

////////////////////////////////////////////////////////////
/// Create a client, connect it to a server, display the
/// welcome message and send an answer.
///
////////////////////////////////////////////////////////////
void runTcpClientReceiver(){
    sf::Packet ToReceive;
    tcpMessage in;
    sf::Socket::Status state = sf::Socket::NotReady;
    while (!quitSocket.load()) {
        state = socket.receive(ToReceive);
        if (state == sf::Socket::Disconnected || state == sf::Socket::Error) {
                //socket disconnected midway thtorug
            quitSocket.store(true);
        
            break;
        }
        tcpMessage temp;
        ToReceive >> temp;
        if (temp.nType == 69) {
            socket.send(ToReceive);
            quitSocket.store(true);
           
            break;
        } else if (state == sf::Socket::Done) {
            // do sending of messaging and stuff here
            messages.push_back(temp);
        }    
    }
    return;
}
void runTcpClient() {

    // TAKING IN CLIENT COMMAND/ INPUT
    std::string commandType;
    int messageType;
    int versionNumber;
    std::string message;
    tcpMessage sentMessage;
    while (!quitSocket.load()) {
        std::cout << "Please enter command: ";
        std::cin >> commandType;
        if (commandType == "v") {
            std::cin >> versionNumber;
            sentMessage.nVersion = static_cast<unsigned char>(versionNumber);
        }  else if (commandType == "q") {
            // break
            // terminate socket connection
            sf::Packet packetQuit;
            tcpMessage quit;
            quit.nType = static_cast<unsigned char>(69);
            packetQuit << quit;
            socket.send(packetQuit);
            quitSocket.store(true);
            break;
        } else if (commandType == "t") {
            std::cin >> messageType;
            std::getline(std::cin, message);
            message = message.substr(1);
            sentMessage.nType = static_cast<unsigned char>(messageType);
            sentMessage.nMsgLen = static_cast<unsigned short>(message.length());
            strncpy(sentMessage.chMsg, message.c_str(), sizeof(sentMessage.chMsg));
            sf::Packet ToSend;
            ToSend << sentMessage;
            sf::Socket::Status senderState = socket.send(ToSend);
            if ( senderState == sf::Socket::Disconnected || senderState == sf::Socket::Error )
                quitSocket.store(true);
            //std::cout << "Message sent to the server: \"" << sentMessage.chMsg << "\"" << std::endl;
            // add small wait after sending message
        } else {
            // reenter command?
            
        }
    }
    
    return; 
}


////////////////////////////////////////////////////////////
/// Entry point of application
///
/// \return Application exit code
///
////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {

    // Choose an arbitrary port for opening sockets

    int port = std::stoi(argv[2]);
    char* hostname = argv[1];
    sf::IpAddress server(hostname);
    while (server == sf::IpAddress::None);
    
    // Connect to the server
    if (socket.connect(server, port) != sf::Socket::Done) {
        std::cout << "failed to connect with server";
        return 1;
    }
    std::cout << "Connected to server " << server << std::endl;
    socket.setBlocking(false);

    std::thread thread1(runTcpClient);
    std::thread thread2(runTcpClientReceiver);

    thread1.join();
    thread2.join();
    
    for (int i = 0; i < messages.size(); i++) {
        std::cout << "Received Msg Type: " << static_cast<int>(((messages[i]).nType)) << "; Msg: " << (messages[i]).chMsg << std::endl ;
    }
    socket.disconnect();
    return EXIT_SUCCESS;
}

