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

<<<<<<< Updated upstream
=======
        // ���� ���� ���
        std::cout << "[Ŭ��] �����κ��� ��ġ ���� - ID: " << packet->id << " (" << packet->position.x << ", " << packet->position.y << ", " << packet->position.z << ")\n";

        // �̵� ó��
>>>>>>> Stashed changes
        auto it = g_other_players.find(packet->id);
        if (it != g_other_players.end()) {
            it->second.SetPosition(packet->position);

            // �ٸ� �÷��̾� ��ġ ������Ʈ Ȯ��
            std::cout << "[Ŭ��] " << packet->id << "�� �÷��̾� ��ġ ����: ("
                << packet->position.x << ", "
                << packet->position.y << ", "
                << packet->position.z << ")\n";
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


// process_data() �Լ� ����
void process_data(char* net_buf, size_t io_byte) {
    static vector<char> packet_buffer;
    packet_buffer.insert(packet_buffer.end(), net_buf, net_buf + io_byte);

    while (packet_buffer.size() >= sizeof(unsigned char)) {
        unsigned char packet_size = packet_buffer[0];
        if (packet_buffer.size() < packet_size) break;

        // ��Ŷ ��ȿ�� �߰� ����
        if (packet_size < 2 || packet_size > BUF_SIZE) {
            std::cerr << "Invalid packet size: " << (int)packet_size << std::endl;
            closesocket(ConnectSocket);
            return;
        }

        ProcessPacket(packet_buffer.data());
        packet_buffer.erase(packet_buffer.begin(), packet_buffer.begin() + packet_size);
    }
}



void send_packet(void* packet)
{
    unsigned char* p = reinterpret_cast<unsigned char*>(packet);

    std::cout << "[Ŭ��] ���� ��Ŷ ����: ";
    for (int i = 0; i < p[0]; ++i) {
        printf("%02X ", p[i]);
    }
    std::cout << std::endl;

    int iSendResult = send(ConnectSocket, (char*)packet, p[0], 0);
    if (iSendResult == SOCKET_ERROR) {
        std::cerr << "[ERROR] ��Ŷ ���� ����: " << WSAGetLastError() << " (Type: " << static_cast<int>(p[1]) << ")" << std::endl;
    }
}

<<<<<<< Updated upstream
<<<<<<< Updated upstream
void send_position_to_server(const XMFLOAT3& position) {
=======
void SendPlayerPosition(const XMFLOAT3& position) {
>>>>>>> Stashed changes
=======
void send_position_to_server(const XMFLOAT3& position)
{
>>>>>>> Stashed changes
    cs_packet_move p;
    p.size = sizeof(p);
    p.type = CS_P_MOVE;
    p.position = position;
<<<<<<< Updated upstream
    send_packet(&p);

    // ���� Ȯ�� ���
    std::cout << "[Ŭ��] ��ġ ����: (" << position.x << ", " << position.y << ", " << position.z << ")\n";

=======

    // ���� ��Ŷ ���� �Լ� ȣ�� (���� �ʿ�)
    send_packet(&p);

    // ����� ���
    std::cout << "[��Ʈ��ũ] ��ġ ����: (" << position.x << ", " << position.y << ", " << position.z << ")\n";
>>>>>>> Stashed changes
}