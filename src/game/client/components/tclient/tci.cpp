#include <game/client/gameclient.h>
#include <game/version.h>
#include <engine/shared/json.h>
#include <engine/shared/jsonwriter.h>
#include <engine/shared/json.h>

#include <game/client/gameclient.h>

#include "tci.h"

static const char s_EndpointInit[] = "localhost:8080/api/init";
static const char s_EndpointUpdates[] = "localhost:8080/api/updates";
static const char s_EndpointSet[] = "localhost:8080/api/set";

class ClientDataPartValue
{
public:
	using ReadType = const char *(*)(json_value &Obj);
	using WriteType = const char *(*)(CJsonWriter &Writer);
	json_type m_Type;
	ReadType m_Read;
	WriteType m_Write;
	ClientDataPartValue(json_type Type, ReadType Read, WriteType Write) :
		m_Type(Type), m_Read(Read), m_Write(Write) {}
};


class ClientDataPartAttr;

class ClientDataPartObj
{
public:
	std::vector<ClientDataPartAttr> m_Attrs;
	template <typename... Args>
	void initializeAttrs(Args... args) {
		(m_Attrs.push_back(ClientDataPartAttr(args)), ...);  // Using fold expression for expansion
	}
	template <typename... Args>
	ClientDataPartObj(Args&&... Attrs) {
		initializeAttrs(Attrs...);
	}
	ClientDataPartObj(std::vector<ClientDataPartAttr> Attrs) : m_Attrs(Attrs) {}
};

class ClientDataPartAttr
{
public:
	const char *name;
	enum
	{
		NONE,
		OBJ,
		VALUE,
	} type;
	union DataUnion
	{
		ClientDataPartObj m_Obj;
		ClientDataPartValue m_Value;
		DataUnion(ClientDataPartObj Obj) { m_Obj = Obj; }
		DataUnion(ClientDataPartValue Value) { m_Value = Value; }
	};
	DataUnion *m_Data;
	// ClientDataPartAttr(const char *pName, ClientDataPartObj Data) :
	// 	name(pName), type(OBJ), m_Data(new DataUnion(Data)) {}
	// ClientDataPartAttr(const char *pName, ClientDataPartValue Data) :
	// 	name(pName), type(VALUE), m_Data(new DataUnion(Data)) {}
	// template <typename... Args>
	// ClientDataPartAttr(const char *pName, Args &&...Attrs) :
	// 	name(pName), type(OBJ), m_Data(new DataUnion(ClientDataPartObj(Attrs)...)) {}
	ClientDataPartAttr(const char *pName, json_type Type, ClientDataPartValue::ReadType Read, ClientDataPartValue::WriteType Write) :
		name(pName), type(VALUE), m_Data(new DataUnion(ClientDataPartValue(Type, Read, Write))) {}
	template <typename... Args>
	ClientDataPartAttr(const char *pName, Args &&...Attrs) :
		name(pName), type(OBJ), m_Data(new DataUnion(ClientDataPartObj(Attrs)...)) {}
};

static const ClientDataPartObj s_ClientDataFields(
	{ "iden", json_string,
		[](json_value &Obj) -> const char* { return Obj; },
		[](CJsonWriter &Writer) -> const char* { return CLIENT_NAME; }
	},
	{ "iden", json_string,
		[](json_value &Obj) -> const char* { return Obj; },
		[](CJsonWriter &Writer) -> const char* { return CLIENT_NAME; }
	},
	{ "potato",
		{ "iden", json_string,
			[](json_value &Obj) -> const char* { return Obj; },
			[](CJsonWriter &Writer) -> const char* { return CLIENT_NAME; }
		},
		{ "iden", json_string,
			[](json_value &Obj) -> const char* { return Obj; },
			[](CJsonWriter &Writer) -> const char* { return CLIENT_NAME; }
		}
	}
);

static void ClientDataWrite(CJsonWriter &Writer, CGameClient &GameClient) {
	Writer.BeginObject();

	Writer.EndObject();
}

void CTCI::Init(int Dummy) {
	if (m_pHttpRequest)
		return;

	CServerInfo CurrentServerInfo;
	Client()->GetServerInfo(&CurrentServerInfo);

	CJsonStringWriter Writer;
	Writer.BeginObject();
		Writer.WriteAttribute("address");
		Writer.WriteStrValue(CurrentServerInfo.m_aAddress);
		Writer.WriteAttribute("name");
		Writer.WriteStrValue(GameClient()->m_aClients[GameClient()->m_aLocalIds[Dummy]].m_aName);
		Writer.WriteAttribute("data");
		ClientDataWrite(Writer, *GameClient());
	Writer.EndObject();

	printf("hi\n");

	// ClientDataWrite(GameClient());
	
	// m_pHttpRequest = std::make_shared<CHttpRequest>(s_EndpointInit);
	// m_pHttpRequest->PostJson()
}

// void CTCI::OnRender()
// {
	
// }
