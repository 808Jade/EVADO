#include "Network.h"
#include "Player.h"


CScene* g_pScene = nullptr;
ID3D12Device* g_pd3dDevice = nullptr;
ID3D12GraphicsCommandList* g_pd3dCommandList = nullptr;
ID3D12RootSignature* g_pd3dGraphicsRootSignature = nullptr;
void* g_pContext = nullptr;

std::unordered_map<long long, CPlayer> g_other_players;
SOCKET ConnectSocket = INVALID_SOCKET;
HANDLE g_hIOCP = INVALID_HANDLE_VALUE;

CPlayer player;
CGameObject object;
long long g_myid = 0;

void PostRecv();


DWORD WINAPI WorkerThread(LPVOID lpParam) {
    while (true) {
        DWORD bytesTransferred = 0;
        ULONG_PTR completionKey = 0;
        OverlappedEx* overlapped = nullptr;

        BOOL result = GetQueuedCompletionStatus(g_hIOCP, &bytesTransferred, &completionKey, reinterpret_cast<LPOVERLAPPED*>(&overlapped), INFINITE);

        if (!result || bytesTransferred == 0) {
            int error_code = result ? WSAGetLastError() : WSAECONNRESET;
            std::cerr << "[Ŭ��] ���� ����. ���� �ڵ�: " << error_code << std::endl;

            // 1. ���� ����
            if (ConnectSocket != INVALID_SOCKET) {
                closesocket(ConnectSocket);
                ConnectSocket = INVALID_SOCKET;
            }

            // 2. �ٸ� �÷��̾� ������ �ʱ�ȭ
            g_other_players.clear();
            g_myid = 0;

            // 3. ������ ���� ���� �˸� (�ɼ�)
            //PostQuitMessage(0); // GUI ���ø����̼��� ���

            delete overlapped;
            continue;
        }

        switch (overlapped->operation) {
        case IO_RECV:
            if (bytesTransferred > 0) {
                process_data(overlapped->buffer, bytesTransferred);
                PostRecv();
            }
            delete overlapped;
            break;
        case IO_SEND:
            delete overlapped;
            break;
        }
    }
    return 0;
}

void PostRecv() {
    if (ConnectSocket == INVALID_SOCKET) return;
    OverlappedEx* overlapped = new OverlappedEx{};
    overlapped->operation = IO_RECV;
    overlapped->wsaBuf.buf = overlapped->buffer;
    overlapped->wsaBuf.len = MAX_PACKET_SIZE;

    DWORD flags = 0;
    WSARecv(ConnectSocket, &overlapped->wsaBuf, 1, nullptr, &flags, reinterpret_cast<LPWSAOVERLAPPED>(overlapped), nullptr);
}

void send_packet(void* packet) {

    if (ConnectSocket == INVALID_SOCKET) return;
    unsigned char* p = reinterpret_cast<unsigned char*>(packet);
    int packet_size = p[0];

    OverlappedEx* overlapped = new OverlappedEx{};
    overlapped->operation = IO_SEND;
    memcpy(overlapped->buffer, packet, packet_size);

    overlapped->wsaBuf.buf = overlapped->buffer;
    overlapped->wsaBuf.len = packet_size;

    int result = WSASend(ConnectSocket, &overlapped->wsaBuf, 1, nullptr, 0, reinterpret_cast<LPWSAOVERLAPPED>(overlapped), nullptr);

    if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        std::cerr << "WSASend ����: " << WSAGetLastError() << std::endl;
        delete overlapped;
    }


}

void InitializeNetwork()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // 1. IOCP �ڵ� ����
    g_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, NUM_WORKER_THREADS);

    // 2. ��Ŀ ������ ����
    for (int i = 0; i < NUM_WORKER_THREADS; ++i) {
        CreateThread(NULL, 0, WorkerThread, NULL, 0, NULL);
    }

    // 3. Overlapped ���� ����
    ConnectSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

    // 4. ����ŷ ���� ����
    u_long nonBlockingMode = 1;
    ioctlsocket(ConnectSocket, FIONBIO, &nonBlockingMode);

    // 5. �񵿱� ���� ����
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
    serverAddr.sin_port = htons(SERVER_PORT);

    // 6. �񵿱� ���� ����
    int connectResult = WSAConnect(ConnectSocket, (sockaddr*)&serverAddr, sizeof(serverAddr), NULL, NULL, NULL, NULL);

    if (connectResult == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err != WSAEWOULDBLOCK) {
            std::cerr << "���� ����: " << err << std::endl;
            closesocket(ConnectSocket);
            WSACleanup();
            exit(1);
        }
    }

    // 7. IOCP�� ���� ���
    CreateIoCompletionPort((HANDLE)ConnectSocket, g_hIOCP, 0, 0);

    // 8. �ʱ� ���� �۾� ����
    PostRecv();

    std::cout << "������ ���������� ����Ǿ����ϴ�." << std::endl;

    // 9. �α��� ��Ŷ ����
    cs_packet_login p;
    p.size = sizeof(p);
    p.type = CS_P_LOGIN;
    p.position = player.GetPosition();
    strcpy_s(p.name, sizeof(p.name), user_name.c_str());
    send_packet(&p);

    std::cout << "[Ŭ��] �α��� ��Ŷ ����: �̸�=" << p.name << " ��ġ(" << p.position.x << "," << p.position.y << "," << p.position.z << ")\n";
}

void ProcessPacket(char* ptr)
{

    const unsigned char packet_type = ptr[1];

    std::cout << "[Ŭ��] ��Ŷ ó�� ���� - Ÿ��: " << (int)packet_type << "\n";

    switch (packet_type)
    {
    case SC_P_USER_INFO: // Ŭ���̾�Ʈ�� ������ ������ �ִ� ��Ŷ Ÿ��
    {
        sc_packet_user_info* packet = reinterpret_cast<sc_packet_user_info*>(ptr);

        std::cout << "[Ŭ��] �� ���� ���� - ID:" << packet->id << " ��ġ(" << packet->position.x << "," << packet->position.y << "," << packet->position.z << ")\n";

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
        std::cout << "[Ŭ��] �� �÷��̾� ����: " << packet->id << std::endl;
        g_pScene->AddRemotePlayer(packet->id, packet->position, g_pd3dDevice, g_pd3dCommandList, g_pd3dGraphicsRootSignature, g_pContext);

        //// �� �÷��̾� ��ü ���� �� �ʱ�ȭ
        //if (g_other_players.find(packet->id) == g_other_players.end()) {
        //    g_other_players[packet->id] = CPlayer{};
        //    std::cout << "[Ŭ��] �� �÷��̾� ����: " << packet->id << std::endl;
        //}
        //g_other_players[packet->id].SetPosition(packet->position);
        break;
    }
    case SC_P_MOVE: // ������ Ŭ���̾�Ʈ���� �ٸ� �÷��̾��� �̵� ������ ������ ��Ŷ Ÿ��
    {
        sc_packet_move* packet = reinterpret_cast<sc_packet_move*>(ptr);
        std::cout << "[Ŭ��] �̵� ��Ŷ ���� - ID: " << packet->id << " (" << packet->position.x << ", " << packet->position.y << ", " << packet->position.z << ")\n";
        g_pScene->UpdateRemotePlayer(packet->id, packet->position);



        // �̵� ó��

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

    while (!packet_buffer.empty()) {
        // 1. �ּ� ��Ŷ ũ�� ����
        if (packet_buffer.size() < sizeof(unsigned char)) break;
        unsigned char packet_size = packet_buffer[0];

        // 2. ��Ŷ ũ�� ��ȿ�� �˻�
        if (packet_size < 2 || packet_size > MAX_PACKET_SIZE) {
            std::cerr << "[Ŭ��] �߸��� ��Ŷ ũ��: " << (int)packet_size << std::endl;
            closesocket(ConnectSocket);
            packet_buffer.clear();
            return;
        }

        // 3. ������ ��Ŷ ���� Ȯ��
        if (packet_buffer.size() < packet_size) break;

        // 4. ��Ŷ ó��
        ProcessPacket(packet_buffer.data());

        // 5. ó���� ������ ����
        packet_buffer.erase(packet_buffer.begin(), packet_buffer.begin() + packet_size);
    }
}


void send_position_to_server(const XMFLOAT3& position)
{

    cs_packet_move p;
    p.size = sizeof(p);
    p.type = CS_P_MOVE;
    p.position = position;

    send_packet(&p);

    // ���� Ȯ�� ���
    std::cout << "[Ŭ��] ��ġ ����: (" << position.x << ", " << position.y << ", " << position.z << ")\n";

}