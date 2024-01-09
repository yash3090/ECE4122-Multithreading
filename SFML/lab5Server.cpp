
////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <iostream>
#include <cstdlib>
#include <SFML/Network.hpp>
#include <cstring>
#include <list>
#include <atomic>
#include <thread>

struct tcpMessage {
    unsigned char nVersion;
    unsigned char nType;
    unsigned short nMsgLen;
    char chMsg[1000];
};

sf::Packet& operator <<(sf::Packet& Packet, const tcpMessage& C)
{
    return Packet << C.nVersion << C.nType << C.nMsgLen << C.chMsg;
}

sf::Packet& operator >>(sf::Packet& Packet, tcpMessage& C)
{
    return Packet >> C.nVersion >> C.nType >> C.nMsgLen >> C.chMsg;
}

// Create a list to store the future clients
std::list<sf::TcpSocket*> clients;

std::string lastMsg;
std::atomic<bool> quitServer(false);

void commandLineInputs() {
    std::string commandType;
    while(!quitServer.load()) {
        std::cout << "Please enter command: ";
        std::cin >> commandType;
        if (commandType.compare("msg") == 0) {
            std::cout << "\nLast Message: " << lastMsg << std::endl;
        } else if (commandType.compare("exit") == 0) {
            quitServer.store(true);
        } else if (commandType.compare("clients") == 0) {
            std::cout << "\nNumber of Clients: " << clients.size() <<std::endl;
            // The listener socket is not ready, test all other sockets (the clients)
            for (std::list<sf::TcpSocket*>::iterator it = clients.begin(); it != clients.end(); ++it) {
                sf::TcpSocket& client = **it;
                std::cout << "\nIP Address: " << client.getRemoteAddress() << " | Port: " << client.getLocalPort() << std::endl;
            }
        }
    }
    return;
}


////////////////////////////////////////////////////////////
/// Launch a server, wait for an incoming connection,
/// send a message and wait for the answer.
///
////////////////////////////////////////////////////////////
void runTcpServer2(int port) {
    // Create a socket to listen to new connections
    sf::TcpListener listener;
    listener.listen(port);
     
    // Create a selector
    sf::SocketSelector selector;

     
    // Add the listener to the selector
    selector.add(listener);

     
// Endless loop that waits for new connections
    while (!quitServer.load())
    {
        // Make the selector wait for data on any socket
        if (selector.wait())
        {   
            // Test the listener
            if (selector.isReady(listener))
            {
                // The listener is ready: there is a pending connection
                sf::TcpSocket* client = new sf::TcpSocket;
                if (listener.accept(*client) == sf::Socket::Done)
                {
                    // Add the new client to the clients list
                    (*client).setBlocking(false);
                    clients.push_back(client);
     
                    // Add the new client to the selector so that we will
                    // be notified when he sends something
                    selector.add(*client);
                }
                else
                {
                    // Error, we won't get a new connection, delete the socket
                    delete client;
                }
            }
            else
            {
                // The listener socket is not ready, test all other sockets (the clients)
                for (std::list<sf::TcpSocket*>::iterator it = clients.begin(); it != clients.end(); ++it)
                {
                    sf::TcpSocket& client = **it;
                    if (selector.isReady(client))
                    {
                        // The client has sent some data, we can receive it
                        sf::Packet packet;
                        if (client.receive(packet) == sf::Socket::Done)
                        {
                            if (quitServer.load()) {
                                for (std::list<sf::TcpSocket*>::iterator it = clients.begin(); it != clients.end(); ++it) {
                                    sf::TcpSocket& client = **it;
                                    if (selector.isReady(client)) {
                                        sf::Packet packetQuit;
                                        tcpMessage quit;
                                        quit.nType = static_cast<unsigned char>(696);
                                        packetQuit << quit;
                                        client.send(packetQuit);
                                        
                                        selector.remove(client);
                                        client.disconnect();
                                        delete(&client);
                                        clients.erase(it);
                                        it--;
                                    }
                                }
                                break;
                            }
                            tcpMessage in;
                            packet >> in;
                       
                      
                            if (in.nType == 69) {
                                
                                sf::Packet packetQuit;
                                tcpMessage quit;
                                quit.nType = 69;
                                packetQuit << quit;
                                client.send(packetQuit);
                                
                                selector.remove(client);
                                client.disconnect();
                                delete(&client);
                                clients.erase(it);
                                it--;
                                continue;
                            }
                            if (in.nVersion == 102) {
                                lastMsg = in.chMsg;
                                if (in.nType == 201) {
                                    std::string reversedString;
                                    // Reverse the original string manually
                                    for (int i = lastMsg.length() - 1; i >= 0; --i) {
                                        reversedString.push_back(lastMsg[i]);
                                    } 
                                    sf::Packet outPacket;
                                    strncpy(in.chMsg, reversedString.c_str(), sizeof(in.chMsg));
                                    outPacket << in;
                                    sf::Socket::Status sendState = client.send(outPacket);
                        
                                } else if (in.nType == 77) {
                                    for (std::list<sf::TcpSocket*>::iterator it2 = clients.begin(); it2 != clients.end(); ++it2) {
                                        sf::TcpSocket& clientBroadcast = **it2;
                                        if (it != it2) {
                                             sf::Socket::Status sendState = clientBroadcast.send(packet);
                                        
                                        }
                                    } 
                                }
                            }
                        }

                    }
                }
            }
        }
    }
    return;
}
void runTcpServer(int port)
{
     
    // Create a selector
    sf::SocketSelector selector;

    sf::TcpListener listener;
    // Listen to the given port for incoming connections
    if (listener.listen(port) != sf::Socket::Done)
        return;
    std::cout << "Server is listening to port " << port << ", waiting for connections... " << std::endl;

    // Add the listener to the selector
    selector.add(listener);

    // Endless loop that waits for new connections
    while (!quitServer.load()) {
        // Make the selector wait for data on any socket
        if (selector.wait())
        {
            // Test the listener
            if (selector.isReady(listener))
            {
                // The listener is ready: there is a pending connection
                sf::TcpSocket* client = new sf::TcpSocket;
                if (listener.accept(*client) == sf::Socket::Done)
                {
                    // Add the new client to the clients list
                    clients.push_back(client);
                    // Add the new client to the selector so that we will
                    // be notified when he sends something
                    selector.add(*client);
                }
                else
                {
                    // Error, we won't get a new connection, delete the socket
                    delete client;
                }
            }
            else
            {
                // The listener socket is not ready, test all other sockets (the clients)
                for (std::list<sf::TcpSocket*>::iterator it = clients.begin(); it != clients.end(); ++it) {
                    sf::TcpSocket& client = **it;
                    if (selector.isReady(client)) {
                        // The client has sent some data, we can receive it
                        sf::Packet packet;
                       
                        if (client.receive(packet) == sf::Socket::Done) {
                            tcpMessage in;
                            packet >> in;
                            if (in.nVersion == 102) {
                                lastMsg = in.chMsg;
                                if (in.nType == 201) {
                                    sf::Socket::Status sendState = client.send(packet);
                                    /**
                                    if (sendState == sf::Socket::Disconnected || sendState == sf::Socket::Error) {
                                        std::cout << "error her";
                                        selector.remove(client);
                                        client.disconnect();
                                        delete(&client);
                                        clients.erase(it);
                                        it--;
                                    std::cout << "Message sent back to the client: \"" << client.getLocalPort()<< "  " << in.chMsg << "\"" << std::endl;
                                    }
                                    **/
                                } else if (in.nType == 77) {
                                    for (std::list<sf::TcpSocket*>::iterator it2 = clients.begin(); it2 != clients.end(); ++it2) {
                                        sf::TcpSocket& clientBroadcast = **it2;
                                        if (it != it2) {
                                             sf::Socket::Status sendState = clientBroadcast.send(packet);
                                             /**
                                            if (sendState != sf::Socket::Done && (sendState == sf::Socket::Disconnected || sendState == sf::Socket::Error)) {
                                                selector.remove(clientBroadcast);
                                                clientBroadcast.disconnect();
                                                delete(&clientBroadcast);
                                                clients.erase(it2);
                                                it2--;
                                             }**/
                                                
                                            //std::cout << "Message sent back to all client: \"" << client.getLocalPort()<< "  " << in.chMsg << "\"" << std::endl;
                                        }

                                    } 

                                }
                            }                    
                        }
                        /**
                        if (state == sf::Socket::Disconnected || state == sf::Socket::Error) {
                            std::cout<<"Client disconnected"<<std::endl;
                            selector.remove(client);
                            client.disconnect();
                            delete(&client);
                            clients.erase(it);
                            it--;
                        }
                        **/
                    }
                }
            }
        }
    return;
    }
}
    /**
    // diff here
    sf::TcpListener listener;
    // Listen to the given port for incoming connections
    if (listener.listen(port) != sf::Socket::Done)
        return;
    // Wait for a connection
    sf::TcpSocket socket;
    if (listener.accept(socket) != sf::Socket::Done)
        return;
    std::cout << "Client connected: " << socket.getRemoteAddress() << std::endl;
    
    
    // Receive a message back from the client
    sf::Packet ToReceive;
    tcpMessage in;
    socket.setBlocking(false);
    sf::Socket::Status state = sf::Socket::NotReady;
    while (state != sf::Socket::Disconnected) { // didnt disconnect port properly gotta chanfge
        state = socket.receive(ToReceive);
        std::cout << " in state " << state << " vs " <<sf::Socket::Done << std::endl;
        while (state != sf::Socket::Done) {
            // wait for recieve to be done
            if (state == sf::Socket::Disconnected || state == sf::Socket::Error) {
                //socket disconnected midway thtorug
                std::cout << "Socket Disconnected  or error";
                break;
            }
            state = socket.receive(ToReceive);
        }
        if (state == sf::Socket::Done) {
            // do sending of messaging and stuff here
            ToReceive >> in;
            std::cout << "Answer received from the client: \"" << in.nType << " " << in.chMsg << "\"" << std::endl;
            // broadcast message

            sf::Packet ToSend;
            ToSend << in;
            if (socket.send(ToSend) != sf::Socket::Done)
                return;
            std::cout << "Message sent to the client: \"" << in.chMsg << "\"" << std::endl;

        } else if (state == sf::Socket::Disconnected) {
            break;
        }
    }
    std::cout << "exited while loop" <<std::endl;

}
**/

////////////////////////////////////////////////////////////
/// Entry point of application
///
/// \return Application exit code
///
////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {

    // Choose an arbitrary port for opening sockets
    int port = std::stoi(argv[1]);

    std::thread thread1(commandLineInputs);
    std::thread thread2(runTcpServer2, port);

    

    thread1.join();
    thread2.join();
    std::cout << " exit main";

    return EXIT_SUCCESS;
}

