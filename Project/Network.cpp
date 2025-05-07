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

WSADATA wsaData;

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
   // p.position = player.GetPosition();
    strcpy_s(p.name, sizeof(p.name), user_name.c_str());
    send_packet(&p);

    std::cout << "[Ŭ��] �α��� ��Ŷ ����: �̸�=" << p.name << "\n";
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
        int id = packet->id;
        std::cout << "[Ŭ��] �� �÷��̾� ����: " << id << std::endl;

        if (id == g_myid) { // �ڽ��� �ƹ�Ÿ ���� �� ī�޶� ��ġ ����            
            std::cout << "[Ŭ��] �� �÷��̾� ����: " << id
                << " (" << packet->name << ")" << std::endl;

            // �ؿ� �̰� �´��� �𸣰���..
            //g_pScene->AddRemotePlayer(packet->id, packet->position, g_pd3dDevice, g_pd3dCommandList, g_pd3dGraphicsRootSignature, g_pContext);

        }
        else if (id < MAX_USER) {  // �ٸ��÷��̾� show()
           
        }
        else { // NPC ���
            std::cout << "[Ŭ��] NPC ����: " << id << std::endl;

        }

        break;
    }
    case SC_P_MOVE: 
    {
        sc_packet_move* packet = reinterpret_cast<sc_packet_move*>(ptr);
        int other_id = packet->id;
        if (other_id == g_myid) { // �ڱ� ��ġ ����
            std::cout << "[Ŭ��] �� ��ġ �̵� - ID: " << other_id << " (" 
                << packet->position.x << ", " 
                << packet->position.y << ", " 
                << packet->position.z << ")\n";


        }
        else if (other_id < MAX_USER) { // �ٸ� �÷��̾� ��ġ ����
            // �ٸ� �÷��̾� ��ġ ������Ʈ Ȯ��
            std::cout << "[Ŭ��] " << other_id << "�� �÷��̾� ��ġ ����: ("
                << packet->position.x << ", "
                << packet->position.y << ", "
                << packet->position.z << ")\n";

            // �ٸ��÷��̾� ���̰� �ϴºκ� �־�� �ҵ�?
            //g_pScene->UpdateRemotePlayer(packet->id, packet->position);

        }
        else { //NPC ��ġ ����
            std::cout << "[Ŭ��] NPC �̵�: " << other_id << std::endl;
        }

        
        break;
    }

    case SC_P_LEAVE: // ������ Ŭ�󿡰� �ٸ� �÷��̾ ������ �������� �˷��ִ� ��Ŷ Ÿ��
    {
        sc_packet_leave* packet = reinterpret_cast<sc_packet_leave*>(ptr);
        int other_id = packet->id;

        if (other_id == g_myid) {
            std::cout << "[Ŭ��] �� �÷��̾� ����: " << other_id << std::endl;

            //����
            // avatar.hide();

        }
        else if (other_id < MAX_USER) {
            std::cout << "[Ŭ��] �÷��̾� ����: " << other_id << std::endl;

            //����
            g_other_players.erase(other_id);
        }
        else {
            std::cout << "[Ŭ��] NPC ����: " << other_id << std::endl;
        }

        
        break;
    }
    default:
        printf("�� �� ���� ��Ŷ Ÿ�� [%d]\n", ptr[1]);
    }
}

// process_data() �Լ� ����
void process_data(char* net_buf, size_t io_byte) {

    char* ptr = net_buf;
    static size_t in_packet_size = 0;
    static size_t saved_packet_size = 0;
    static char packet_buffer[BUF_SIZE];

    while (0 != io_byte) {
        if (0 == in_packet_size) in_packet_size = ptr[0];
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