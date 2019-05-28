/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef ENGINE_CLIENT_CLIENT_H
#define ENGINE_CLIENT_CLIENT_H

#include <memory>

#include <base/hash.h>
#include <engine/client/http.h>

#define CONNECTLINK "ddnet:"

class CGraph
{
public:
	enum
	{
		// restrictions: Must be power of two
		MAX_VALUES=128,
	};

	float m_Min, m_Max;
	float m_aValues[MAX_VALUES];
	float m_aColors[MAX_VALUES][3];
	int m_Index;

	void Init(float Min, float Max);

	void ScaleMax();
	void ScaleMin();

	void Add(float v, float r, float g, float b);
	void Render(IGraphics *pGraphics, int Font, float x, float y, float w, float h, const char *pDescription);
};


class CSmoothTime
{
	int64 m_Snap;
	int64 m_Current;
	int64 m_Target;

	CGraph m_Graph;

	int m_SpikeCounter;

	float m_aAdjustSpeed[2]; // 0 = down, 1 = up
public:
	void Init(int64 Target);
	void SetAdjustSpeed(int Direction, float Value);

	int64 Get(int64 Now);

	void UpdateInt(int64 Target);
	void Update(CGraph *pGraph, int64 Target, int TimeLeft, int AdjustDirection);
};

class CDemoEdit : public IJob
{
	IStorage *m_pStorage;
	IConsole *m_pConsole;
	CDemoEditor m_DemoEditor;
	char m_pDemo[256];
	char m_pDst[256];
	int m_StartTick;
	int m_EndTick;

public:
	CDemoEdit(const char *pNetVersion, CSnapshotDelta *pSnapshotDelta, IConsole *pConsole, IStorage *pStorage, const char *pDemo, const char *pDst, int StartTick, int EndTick);
	~CDemoEdit();
	void Run();
};

class CClient : public IClient, public CDemoPlayer::IListener
{
	// needed interfaces
	IEngine *m_pEngine;
	IEditor *m_pEditor;
	IEngineInput *m_pInput;
	IEngineGraphics *m_pGraphics;
	IEngineSound *m_pSound;
	IGameClient *m_pGameClient;
	IEngineMap *m_pMap;
	IConsole *m_pConsole;
	IStorage *m_pStorage;
	IUpdater *m_pUpdater;
	IEngineMasterServer *m_pMasterServer;

	enum
	{
		NUM_SNAPSHOT_TYPES=2,
		PREDICTION_MARGIN=1000/50/2, // magic network prediction value
	};

	class CNetClient m_NetClient[3];
	class CDemoPlayer m_DemoPlayer;
	class CDemoRecorder m_DemoRecorder[RECORDER_MAX];
	class CDemoEditor m_DemoEditor;
	class CGhostRecorder m_GhostRecorder;
	class CGhostLoader m_GhostLoader;
	class CServerBrowser m_ServerBrowser;
	class CUpdater m_Updater;
	class CFriends m_Friends;
	class CFriends m_Foes;

	char m_aServerAddressStr[256];

	unsigned m_SnapshotParts[2];
	int64 m_LocalStartTime;

	int m_DebugFont;

	int64 m_LastRenderTime;
	float m_RenderFrameTimeLow;
	float m_RenderFrameTimeHigh;
	int m_RenderFrames;

	NETADDR m_ServerAddress;
	int m_SnapCrcErrors;
	bool m_AutoScreenshotRecycle;
	bool m_AutoStatScreenshotRecycle;
	bool m_AutoCSVRecycle;
	bool m_EditorActive;
	bool m_SoundInitFailed;
	bool m_ResortServerBrowser;

	int m_AckGameTick[2];
	int m_CurrentRecvTick[2];
	int m_RconAuthed[2];
	char m_RconPassword[32];
	int m_UseTempRconCommands;
	char m_Password[32];
	bool m_SendPassword;

	// version-checking
	char m_aVersionStr[10];

	// pinging
	int64 m_PingStartTime;

	char m_aCurrentMap[MAX_PATH_LENGTH];
	char m_aCurrentMapPath[MAX_PATH_LENGTH];

	char m_aTimeoutCodes[2][32];
	bool m_aTimeoutCodeSent[2];
	bool m_GenerateTimeoutSeed;

	//
	char m_aCmdConnect[256];
	char m_aCmdPlayDemo[MAX_PATH_LENGTH];

	// map download
	std::shared_ptr<CGetFile> m_pMapdownloadTask;
	char m_aMapdownloadFilename[256];
	char m_aMapdownloadName[256];
	IOHANDLE m_MapdownloadFile;
	int m_MapdownloadChunk;
	int m_MapdownloadCrc;
	int m_MapdownloadAmount;
	int m_MapdownloadTotalsize;
	bool m_MapdownloadSha256Present;
	SHA256_DIGEST m_MapdownloadSha256;

	bool m_MapDetailsPresent;
	char m_aMapDetailsName[256];
	int m_MapDetailsCrc;
	SHA256_DIGEST m_MapDetailsSha256;

	std::shared_ptr<CGetFile> m_pDDNetInfoTask;

	// time
	CSmoothTime m_GameTime[2];
	CSmoothTime m_PredictedTime;

	// input
	struct // TODO: handle input better
	{
		int m_aData[MAX_INPUT_SIZE]; // the input data
		int m_Tick; // the tick that the input is for
		int64 m_PredictedTime; // prediction latency when we sent this input
		int64 m_Time;
	} m_aInputs[2][200];

	int m_CurrentInput[2];
	bool m_LastDummy;
	bool m_LastDummy2;
	bool m_DummySendConnInfo;

	// graphs
	CGraph m_InputtimeMarginGraph;
	CGraph m_GametimeMarginGraph;
	CGraph m_FpsGraph;

	// the game snapshots are modifiable by the game
	class CSnapshotStorage m_SnapshotStorage[2];
	CSnapshotStorage::CHolder *m_aSnapshots[2][NUM_SNAPSHOT_TYPES];

	int m_ReceivedSnapshots[2];
	char m_aSnapshotIncomingData[CSnapshot::MAX_SIZE];

	class CSnapshotStorage::CHolder m_aDemorecSnapshotHolders[NUM_SNAPSHOT_TYPES];
	char *m_aDemorecSnapshotData[NUM_SNAPSHOT_TYPES][2][CSnapshot::MAX_SIZE];

	class CSnapshotDelta m_SnapshotDelta;

	std::list<std::shared_ptr<CDemoEdit>> m_EditJobs;

	//
	class CServerInfo m_CurrentServerInfo;
	int64 m_CurrentServerInfoRequestTime; // >= 0 should request, == -1 got info

	// version info
	struct CVersionInfo
	{
		enum
		{
			STATE_INIT=0,
			STATE_START,
			STATE_READY,
		};

		int m_State;
		class CHostLookup m_VersionServeraddr;
	} m_VersionInfo;

	volatile int m_GfxState;
	static void GraphicsThreadProxy(void *pThis) { ((CClient*)pThis)->GraphicsThread(); }
	void GraphicsThread();

#if defined(CONF_FAMILY_UNIX)
	CFifo m_Fifo;
#endif

public:
	IEngine *Engine() { return m_pEngine; }
	IEngineGraphics *Graphics() { return m_pGraphics; }
	IEngineInput *Input() { return m_pInput; }
	IEngineSound *Sound() { return m_pSound; }
	IGameClient *GameClient() { return m_pGameClient; }
	IEngineMasterServer *MasterServer() { return m_pMasterServer; }
	IStorage *Storage() { return m_pStorage; }
	IUpdater *Updater() { return m_pUpdater; }

	CClient();

	// ----- send functions -----
	virtual int SendMsg(CMsgPacker *pMsg, int Flags);
	virtual int SendMsgExY(CMsgPacker *pMsg, int Flags, bool System=true, int NetClient=1);

	int SendMsgEx(CMsgPacker *pMsg, int Flags, bool System=true);
	void SendInfo();
	void SendEnterGame();
	void SendReady();
	void SendMapRequest();

	virtual bool RconAuthed() { return m_RconAuthed[g_Config.m_ClDummy] != 0; }
	virtual bool UseTempRconCommands() { return m_UseTempRconCommands != 0; }
	void RconAuth(const char *pName, const char *pPassword);
	virtual void Rcon(const char *pCmd);

	virtual bool ConnectionProblems();

	virtual bool SoundInitFailed() { return m_SoundInitFailed; }

	virtual int GetDebugFont() { return m_DebugFont; }

	void DirectInput(int *pInput, int Size);
	void SendInput();

	// TODO: OPT: do this a lot smarter!
	virtual int *GetInput(int Tick);
	virtual int *GetDirectInput(int Tick);

	const char *LatestVersion();

	// ------ state handling -----
	void SetState(int s);

	// called when the map is loaded and we should init for a new round
	void OnEnterGame();
	virtual void EnterGame();

	virtual void Connect(const char *pAddress, const char *pPassword = NULL);
	void DisconnectWithReason(const char *pReason);
	virtual void Disconnect();

	virtual void DummyDisconnect(const char *pReason);
	virtual void DummyConnect();
	virtual bool DummyConnected();
	virtual bool DummyConnecting();
	int m_DummyConnected;
	int m_LastDummyConnectTime;

	virtual void GetServerInfo(CServerInfo *pServerInfo);
	void ServerInfoRequest();

	int LoadData();

	// ---

	int GetPredictionTime();
	void *SnapGetItem(int SnapID, int Index, CSnapItem *pItem);
	void SnapInvalidateItem(int SnapID, int Index);
	void *SnapFindItem(int SnapID, int Type, int ID);
	int SnapNumItems(int SnapID);
	void SnapSetStaticsize(int ItemType, int Size);

	void Render();
	void DebugRender();

	virtual void Restart();
	virtual void Quit();

	virtual const char *ErrorString();

	const char *LoadMap(const char *pName, const char *pFilename, SHA256_DIGEST *pWantedSha256, unsigned WantedCrc);
	const char *LoadMapSearch(const char *pMapName, SHA256_DIGEST *pWantedSha256, int WantedCrc);

	static int PlayerScoreNameComp(const void *a, const void *b);

	void ProcessConnlessPacket(CNetChunk *pPacket);
	void ProcessServerInfo(int Type, NETADDR *pFrom, const void *pData, int DataSize);
	void ProcessServerPacket(CNetChunk *pPacket);
	void ProcessServerPacketDummy(CNetChunk *pPacket);

	void ResetMapDownload();
	void FinishMapDownload();

	void RequestDDNetInfo();
	void ResetDDNetInfo();
	void FinishDDNetInfo();
	void LoadDDNetInfo();

	virtual const char *MapDownloadName() { return m_aMapdownloadName; }
	virtual int MapDownloadAmount() { return !m_pMapdownloadTask ? m_MapdownloadAmount : (int)m_pMapdownloadTask->Current(); }
	virtual int MapDownloadTotalsize() { return !m_pMapdownloadTask ? m_MapdownloadTotalsize : (int)m_pMapdownloadTask->Size(); }

	void PumpNetwork();

	virtual void OnDemoPlayerSnapshot(void *pData, int Size);
	virtual void OnDemoPlayerMessage(void *pData, int Size);

	void Update();

	void RegisterInterfaces();
	void InitInterfaces();

	void Run();

	bool CtrlShiftKey(int Key, bool &Last);

	static void Con_Connect(IConsole::IResult *pResult, void *pUserData);
	static void Con_Disconnect(IConsole::IResult *pResult, void *pUserData);

	static void Con_DummyConnect(IConsole::IResult *pResult, void *pUserData);
	static void Con_DummyDisconnect(IConsole::IResult *pResult, void *pUserData);

	static void Con_Quit(IConsole::IResult *pResult, void *pUserData);
	static void Con_DemoPlay(IConsole::IResult *pResult, void *pUserData);
	static void Con_DemoSpeed(IConsole::IResult *pResult, void *pUserData);
	static void Con_Minimize(IConsole::IResult *pResult, void *pUserData);
	static void Con_Ping(IConsole::IResult *pResult, void *pUserData);
	static void Con_Screenshot(IConsole::IResult *pResult, void *pUserData);
	static void Con_Rcon(IConsole::IResult *pResult, void *pUserData);
	static void Con_RconAuth(IConsole::IResult *pResult, void *pUserData);
	static void Con_RconLogin(IConsole::IResult *pResult, void *pUserData);
	static void Con_AddFavorite(IConsole::IResult *pResult, void *pUserData);
	static void Con_RemoveFavorite(IConsole::IResult *pResult, void *pUserData);
	static void Con_Play(IConsole::IResult *pResult, void *pUserData);
	static void Con_Record(IConsole::IResult *pResult, void *pUserData);
	static void Con_StopRecord(IConsole::IResult *pResult, void *pUserData);
	static void Con_AddDemoMarker(IConsole::IResult *pResult, void *pUserData);
	static void ConchainServerBrowserUpdate(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);
	static void ConchainFullscreen(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);
	static void ConchainWindowBordered(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);
	static void ConchainWindowScreen(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);
	static void ConchainWindowVSync(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);
	static void ConchainTimeoutSeed(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);
	static void ConchainPassword(IConsole::IResult *pResult, void *pUserData, IConsole::FCommandCallback pfnCallback, void *pCallbackUserData);

	static void Con_DemoSlice(IConsole::IResult *pResult, void *pUserData);
	static void Con_DemoSliceBegin(IConsole::IResult *pResult, void *pUserData);
	static void Con_DemoSliceEnd(IConsole::IResult *pResult, void *pUserData);
	static void Con_SaveReplay(IConsole::IResult *pResult, void *pUserData);

	void RegisterCommands();

	const char *DemoPlayer_Play(const char *pFilename, int StorageType);
	void DemoRecorder_Start(const char *pFilename, bool WithTimestamp, int Recorder);
	void DemoRecorder_HandleAutoStart();
	void DemoRecorder_StartReplayRecorder();
	void DemoRecorder_Stop(int Recorder, bool RemoveFile = false);
	void DemoRecorder_AddDemoMarker(int Recorder);
	class IDemoRecorder *DemoRecorder(int Recorder);

	void AutoScreenshot_Start();
	void AutoStatScreenshot_Start();
	void AutoScreenshot_Cleanup();
	void AutoStatScreenshot_Cleanup();

	void AutoCSV_Start();
	void AutoCSV_Cleanup();

	void ServerBrowserUpdate();

	void HandleConnectLink(const char *pLink);
	void HandleDemoPath(const char *pPath);

	// gfx
	void SwitchWindowScreen(int Index);
	void ToggleFullscreen();
	void ToggleWindowBordered();
	void ToggleWindowVSync();
	void LoadFont();

	// DDRace

	void GenerateTimeoutSeed();
	void GenerateTimeoutCodes();

	virtual int GetCurrentRaceTime();

	const char *GetCurrentMap();
	const char *GetCurrentMapPath();
	unsigned GetMapCrc();

	void RaceRecord_Start(const char *pFilename);
	void RaceRecord_Stop();
	bool RaceRecord_IsRecording();

	virtual void DemoSliceBegin();
	virtual void DemoSliceEnd();
	virtual void DemoSlice(const char *pDstPath, CLIENTFUNC_FILTER pfnFilter, void *pUser);
	virtual void SaveReplay();

	virtual void Notify(const char * pTitle, const char * pMessage);

	bool EditorHasUnsavedData() { return m_pEditor->HasUnsavedData(); }

	virtual IFriends* Foes() {return &m_Foes; }
	virtual void EndNotification();

	void GetSmoothTick(int *pSmoothTick, float *pSmoothIntraTick, float MixAmount);
};

#endif
