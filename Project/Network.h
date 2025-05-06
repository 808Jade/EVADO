#pragma once
#include "Common.h"
#include "Object.h"
#include "Player.h"
#include "Scene.h"

// Ŭ���̾�Ʈ���� ������ �����Ұ� -----------
// 
// �÷��̾��� ��ǥ(x,y,z)
// �÷��̾��� id(user_name)
// 
// 
// ������Ʈ�� ��ǥ(ob_x,ob_y,ob_z)
// 
// 
//  <�ʿ��Ѱ�>
//		�÷��̾��� ���� �������
//		������Ʈ�� ���� �������
// 
// ----------------------------------------


enum IO_OPERATION { IO_RECV, IO_SEND, IO_CONNECT };

struct OverlappedEx {
    WSAOVERLAPPED overlapped;
    WSABUF wsaBuf;
    char buffer[MAX_PACKET_SIZE];
    IO_OPERATION operation;
    int bytesTransferred;
};


extern HANDLE g_hIOCP;
extern SOCKET ConnectSocket;
extern std::unordered_map<long long, CPlayer> g_other_players;
extern long long g_myid;
extern std::string user_name;

extern CScene* g_pScene;
extern ID3D12Device* g_pd3dDevice;
extern ID3D12GraphicsCommandList* g_pd3dCommandList;
extern ID3D12RootSignature* g_pd3dGraphicsRootSignature;
extern void* g_pContext;


void ProcessPacket(char* ptr);
void process_data(char* net_buf, size_t io_byte);
void send_packet(void* packet);
void send_position_to_server(const XMFLOAT3& position);
void InitializeNetwork();


