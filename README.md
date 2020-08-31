# Client-and-server-components-of-a-text-chat-application

Developed the client and server components of a text chat application, consisting of one chat server and multiple chat clients over TCP connections

The chat application follows a typical client-server model, whereby we will have one server instance and two or more client instances. Given that we will be testing on the five dedicated hosts listed before, you can assume that at most four clients will be online at any given time.

The clients, when launched, log in to the server, identify themselves, and obtain the list of other clients that are connected to the server. Clients can either send a unicast message to any one of the other clients or a broadcast a message to all the other clients. The clients maintain an active connection only with the server and not with any other clients. Clients never exchange messages directly with each other.

The server exists to facilitate the exchange of messages between the clients. The server can exchange control messages with the clients. Among other things, it maintains a list of all clients that are connected to it, and their related information. The server stores/buffers any messages destined to clients that are not logged-in at the time of the receipt of the message at the server from the sender, to be delivered at a later time when the client logs in to the server.
