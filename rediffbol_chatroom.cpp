/**
 * @file rediffbol_chatroom.cpp 
 * 
 * Copyright (C) 2008-2009 Arnold Noronha <arnstein87 AT gmail DOT com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version. 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include "rediffbol.h" 
#include <string> 
#include <sstream>
#include "util.h"

using namespace std;
using namespace rbol;

PurpleRoomlist* RediffBolConn::sendGetChatRoomsRequest(){
	int totalsize = 24+strlen(CSRequestHeader)+strlen(CSCmdGetChatRooms);
	ostringstream out;
	out<<intToDWord(totalsize-4);
	out<<intToDWord(0);
	out<<intToDWord(7);
	
	out<<intToDWord(strlen(CSRequestHeader));
	out<<CSRequestHeader;
	out<<intToDWord(strlen(CSCmdGetChatRooms));
	out<<CSCmdGetChatRooms;
	out<<intToDWord(0);
	
	hex_dump(out.str(), "GetChatRoomsRequest");
	connection->write(out.str());


	if (roomlist) { 
		purple_roomlist_unref(roomlist);
		roomlist = NULL;
	}

	PurpleRoomlist* rl = purple_roomlist_new(account);
	GList* fields = NULL;
	PurpleRoomlistField * f = purple_roomlist_field_new (PURPLE_ROOMLIST_FIELD_STRING, "", "room", FALSE);
	fields = g_list_append(fields, f);
	
	f = purple_roomlist_field_new(PURPLE_ROOMLIST_FIELD_STRING, "", 
				      "id", FALSE);

	fields = g_list_append(fields, f);
	purple_roomlist_set_fields(rl, fields);

	
	purple_roomlist_set_in_progress(rl, TRUE);

	return (this->roomlist = rl);
}

void RediffBolConn::parseChatRoomsResponse(MessageBuffer &buffer) {
	int payloadsize = buffer.readInt();
	buffer = buffer.readMessageBuffer(payloadsize);

	int numlobbies = buffer.readInt();

	vector<Lobby> lobbylist;
	int totalusers = 0;
	
	for(int i = 0;i < numlobbies ; i++){
		string lobbyname = buffer.readString();
		Lobby lobby;
		lobby.occupants = 0;
		lobby.name = lobbyname;
		int code = buffer.readInt();
		string roomname = buffer.readString();
		int numrooms = buffer.readInt();
		

		PurpleRoomlistRoom *r = purple_roomlist_room_new(
			PURPLE_ROOMLIST_ROOMTYPE_ROOM ,
			roomname.c_str(), NULL);
			
		assert(r);
		purple_roomlist_room_add_field(roomlist, r, roomname.c_str());
		purple_roomlist_room_add_field(roomlist, r, roomname.c_str());

		purple_roomlist_room_add(roomlist, r);

		for(int i = 0;i < numrooms; i++) {
			ChatRoom room;
			room.number = buffer.readInt();
			room.occupants = buffer.readInt();
			lobby.occupants += room.occupants;
		
			purple_debug_info("rbol", "Got room: %s %d %d\n",
					  roomname.c_str(), room.number, 
					  room.occupants);

			/*
			 * Add the room. Purple work.
			 */

			PurpleRoomlistRoom *l = purple_roomlist_room_new(
				PURPLE_ROOMLIST_ROOMTYPE_ROOM,
				roomname.c_str(),  NULL);
			
			room.name = roomname;

			purple_roomlist_room_add_field(roomlist, l, 
						       roomname.c_str());
			purple_roomlist_room_add_field(roomlist, l, 
						       roomname.c_str());
			
			purple_roomlist_room_add(roomlist, l);
			lobby.subrooms.push_back(room);
		}
		
		lobbylist.push_back(lobby);
		totalusers += lobby.occupants;
		
	}

	purple_roomlist_set_in_progress(roomlist, FALSE);
}
