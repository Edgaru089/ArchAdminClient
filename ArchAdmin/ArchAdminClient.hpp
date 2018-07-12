#pragma once

#include <string>
#include <SFML/Network.hpp>
#include "SHA-256.hpp"
#include "Uuid.hpp"
using namespace std;
using namespace sf;

class ArchAdminClient {
public:
	ArchAdminClient() :connected(false) {}

	static string getHashOfPassword(string passwordCleartext) { return sha256(passwordCleartext); }

	bool connect(IpAddress targetIp, Uint16 targetPort, string passwordHashed) {
		this->passwordHashed = passwordHashed;

		Socket::Status stat = socket.connect(targetIp, targetPort);
		if (stat != Socket::Done)
			return false;

		Packet pack;
		pack << "ADMINLOGIN" << passwordHashed;
		socket.send(pack);

		pack.clear();
		stat = socket.receive(pack);
		if (stat != Socket::Done) {
			socket.disconnect();
			return false;
		}

		string result;
		if (!(pack >> result) || result != "ADMINOK") {
			socket.disconnect();
			return false;
		}

		connected = true;
		return true;
	}

	void disconnect() {
		socket.disconnect();
		connected = false;
	}

	bool isConnected() { return connected; }

#define CHECKED_RECEIVE(packetName) \
Packet packetName; {Socket::Status stat = socket.receive(packetName); if (stat != Socket::Done){connected = false; return false;} }
#define PACKET_COMMAND(packetName, command) \
Packet packetName; packetName << command;
#define CHECKED_SEND(packetName) \
{Socket::Status stat = socket.send(packetName); if (stat != Socket::Done){connected = false; return false;} }
#define CHECKED_SEND_COMMAND(command)\
{PACKET_COMMAND(pack, command); CHECKED_SEND(pack); }

	// Username, <Password, SessionOrNil>
	bool listUsers(vector<pair<string, pair<string, Uuid>>>& vec) {
		CHECKED_SEND_COMMAND("A_LISTUSERS");
		CHECKED_RECEIVE(ret);
		string reply;
		ret >> reply;
		if (reply == "A_USERS") {
			int cnt;
			string name, pass;
			Uuid sess;
			ret >> cnt;
			vec.clear(); vec.reserve(cnt);
			for (int i = 1; i <= cnt; i++) {
				ret >> name >> pass >> sess;
				vec.push_back(make_pair(name, make_pair(pass, sess)));
			}
			return true;
		}
		else
			return false;
	}

	bool deleteUser(string name) {
		PACKET_COMMAND(pack, "A_DELETEUSER");
		pack << name;
		CHECKED_SEND(pack);
		CHECKED_RECEIVE(ret);
		string reply;
		ret >> reply;
		if (reply == "A_USERDELETED")
			return true;
		else
			return false;
	}

	bool addUser(string name, string passHashed) {
		PACKET_COMMAND(pack, "A_ADDUSER");
		pack << name << passHashed;
		CHECKED_SEND(pack);
		CHECKED_RECEIVE(ret);
		string reply;
		ret >> reply;
		if (reply == "A_USERADDED")
			return true;
		else
			return false;
	}

	bool changeUser(string name, string newPasswordHashed) {
		PACKET_COMMAND(pack, "A_CHANGEUSER");
		pack << name << newPasswordHashed;
		CHECKED_SEND(pack);
		CHECKED_RECEIVE(ret);
		string reply;
		ret >> reply;
		if (reply == "A_USERCHANGED")
			return true;
		else
			return false;
	}

	bool removeSession(string name) {
		PACKET_COMMAND(pack, "A_REMOVESESS");
		pack << name;
		CHECKED_SEND(pack);
		CHECKED_RECEIVE(ret);
		string reply;
		ret >> reply;
		if (reply == "A_SESSREMOVED")
			return true;
		else
			return false;
	}

	bool acquireSession(string name, Uuid& session) {
		PACKET_COMMAND(pack, "A_ADDSESS");
		pack << name;
		CHECKED_SEND(pack);
		CHECKED_RECEIVE(ret);
		string reply;
		ret >> reply;
		if (reply == "A_SESSADDED") {
			ret >> session;
			return true;
		}
		else
			return false;
	}

private:
	bool connected;
	TcpSocket socket;

	string passwordHashed;
};

