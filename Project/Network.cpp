#include "Network.h"
#include "Player.h"



std::unordered_map<long long, CPlayer> g_other_players;
SOCKET ConnectSocket = INVALID_SOCKET;

CPlayer player;
CGameObject object;


long long g_myid = 0;

void ProcessPacket(char* ptr)
{

    const unsigned char packet_type = ptr[1];

#ifdef _DEBUG
    std::cout << "[PROCESS] ��Ŷ Ÿ��: " << static_cast<int>(packet_type) << std::endl;
#endif

    switch (packet_type)
    {
    case SC_P_USER_INFO: // Ŭ���̾�Ʈ�� ������ ������ �ִ� ��Ŷ Ÿ��
    {
        sc_packet_user_info* packet = reinterpret_cast<sc_packet_user_info*>(ptr);

        // ��Ŷ ũ�� ���� �߰�
        if (packet->size != sizeof(sc_packet_user_info)) {
            std::cerr << "[ERROR] �߸��� USER_INFO ��Ŷ: "
                << packet->size << " vs " << sizeof(sc_packet_user_info) << std::endl;
            closesocket(ConnectSocket);
            return;
        }

        g_myid = packet->id;
        player.SetPosition(packet->position);

        if (player.GetCamera()) {
            player.GetCamera()->SetPosition(packet->position);
        }
        break;
    }

    case SC_P_LOGIN_FAIL:  // �α��� ���� ó��
    {
        std::cerr << "�α��� ����: �߸��� ��й�ȣ�Դϴ�." << std::endl;
        closesocket(ConnectSocket);
        exit(1);
        break;
    }

    case SC_P_ENTER: // ���� ���� �÷��̾��� ������ �����ϰ� �ִ� ��Ŷ Ÿ��
    {
        sc_packet_enter* packet = reinterpret_cast<sc_packet_enter*>(ptr);

        // o_type �ʵ� ó�� �߰�
        if (packet->o_type != 0) { // 0=�÷��̾�, 1=NPC ��
            std::cout << "NPC ����: Ÿ�� " << (int)packet->o_type << std::endl;
            return;
        }

        CPlayer new_player;

        // �ش� �÷��̾��� ��ġ�� ��Ŷ�� ���Ե� ��ǥ�� �����ϴ� �ڵ� �߰�
        new_player.SetPosition(packet->position);

        g_other_players[packet->id] = new_player;
        std::cout << "�� �÷��̾� ����: " << packet->name << " (ID:" << packet->id << ")" << std::endl;

        break;
    }
    case SC_P_MOVE: // ������ Ŭ���̾�Ʈ���� �ٸ� �÷��̾��� �̵� ������ ������ ��Ŷ Ÿ��
    {
        sc_packet_move* packet = reinterpret_cast<sc_packet_move*>(ptr);
        
        // ���� ���� ���
        std::cout << "[Ŭ��] �����κ��� ��ġ ���� - ID: " << packet->id
            << " (" << packet->position.x << ", " << packet->position.y
            << ", " << packet->position.z << ")\n";

        // �̵� ó��
        auto it = g_other_players.find(packet->id);
        if (it != g_other_players.end()) {
            // �ε巯�� �̵��� ���� �ӵ� ��� ������Ʈ
            XMFLOAT3 target = packet->position;
            XMFLOAT3 current = it->second.GetPosition();
            XMFLOAT3 velocity = { // �̺κ��� Ŭ��� �̾߱� �ϸ鼭? �ϸ� �ɵ�
                (packet->position.x - it->second.GetPosition().x) * 10.0f,
                (packet->position.y - it->second.GetPosition().y) * 10.0f,
                (packet->position.z - it->second.GetPosition().z) * 10.0f
            };
            it->second.SetVelocity(velocity);
        }


        break;
    }

    case SC_P_LEAVE: // ������ Ŭ�󿡰� �ٸ� �÷��̾ ������ �������� �˷��ִ� ��Ŷ Ÿ��
    {
        sc_packet_leave* packet = reinterpret_cast<sc_packet_leave*>(ptr);

        // �÷��̾� ���� �� ���ҽ� ����
        auto it = g_other_players.find(packet->id);
        if (it != g_other_players.end()) {
            it->second.ReleaseShaderVariables(); // �׷��� ���ҽ� ����
            g_other_players.erase(it);

            // �÷��̾� ���� �˸�
            std::cout << "�÷��̾� ����: ID=" << packet->id << std::endl;
        }
        break;
    }
    default:
        printf("�� �� ���� ��Ŷ Ÿ�� [%d]\n", ptr[1]);
        closesocket(ConnectSocket);
        exit(1);
    }
}


void process_data(char* net_buf, size_t io_byte)
{

    // ��Ŷ ó�� ���� �α�
#ifdef _DEBUG
    std::cout << "[RECV] ���� ������ ũ��: " << io_byte << " bytes" << std::endl;
#endif

    char* ptr = net_buf;
    static size_t in_packet_size = 0;
    static size_t saved_packet_size = 0;
    static char packet_buffer[BUF_SIZE];

    while (0 != io_byte) {
        if (0 == in_packet_size) in_packet_size = ptr[0];
        if (in_packet_size > BUF_SIZE) {
            std::cerr << "[ERROR] ��Ŷ ������ �ʰ�: " << in_packet_size
                << "/" << BUF_SIZE << " (Connection Closed)" << std::endl;
            closesocket(ConnectSocket);
            return;
        }
        if (io_byte + saved_packet_size >= in_packet_size) {
            memcpy(packet_buffer + saved_packet_size, ptr, in_packet_size - saved_packet_size);
            ProcessPacket(packet_buffer);
            ptr += in_packet_size - saved_packet_size;
            io_byte -= in_packet_size - saved_packet_size;
            in_packet_size = 0;
            saved_packet_size = 0;
        }
        else {
            memcpy(packet_buffer + saved_packet_size, ptr, io_byte);
            saved_packet_size += io_byte;
            io_byte = 0;
        }
    }
}

void send_packet(void* packet)
{
    unsigned char* p = reinterpret_cast<unsigned char*>(packet);

#ifdef _DEBUG
    std::cout << "[SEND] ��Ŷ Ÿ��: " << static_cast<int>(p[1])
        << ", ũ��: " << static_cast<int>(p[0]) << std::endl;
#endif

    int iSendResult = send(ConnectSocket, (char*)packet, p[0], 0);
    if (iSendResult == SOCKET_ERROR) {
        std::cerr << "[ERROR] ��Ŷ ���� ����: " << WSAGetLastError() << " (Type: " << static_cast<int>(p[1]) << ")" << std::endl;
    }
}

void send_position_to_server(const XMFLOAT3& position) {
    cs_packet_move p;
    p.size = sizeof(p);
    p.type = CS_P_MOVE;
    p.position = position;
    send_packet(&p);
}