#pragma once
#include "Common.h"
#include "Object.h"
#include "Player.h"

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




extern SOCKET ConnectSocket;

void ProcessPacket(char* ptr);
void process_data(char* net_buf, size_t io_byte);
void send_packet(void* packet);
<<<<<<< Updated upstream
void send_position_to_server(const XMFLOAT3& position);
=======
void SendPlayerPosition(const DirectX::XMFLOAT3& position);
>>>>>>> Stashed changes

// ��Ŷ ���� �Լ� ����
extern void send_position_to_server(const DirectX::XMFLOAT3& position);


//extern OBJECT monster, user;
//std::vector<OBJECT> players[MAX_USER];
extern std::unordered_map<long long, CPlayer> g_other_players;
extern long long g_myid;
extern std::string user_name;