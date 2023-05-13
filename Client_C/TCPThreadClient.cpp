#include "Common.h"
#include <process.h>
#include <windows.h>


#pragma comment(lib, "ws2_32") 

char* SERVERIP = (char*)"127.0.0.1";
#define SERVERPORT 9900
#define BUFSIZE    1023
#define NAMESIZE   1023


unsigned int WINAPI ThreadSend(void* arg) {
	SOCKET sock = *(SOCKET*)arg;
	int len;
	char buf[BUFSIZE + 1];
	int retval;

	while (1) {

		// ������ �Է�
		if (fgets(buf, BUFSIZE + 1, stdin) == NULL)
			break;

		len = (int)strlen(buf);
		if (buf[len - 1] == '\n')
			buf[len - 1] = '\0';

		// ������ ������
		retval = send(sock, buf, (int)strlen(buf), 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
		else if (strcmp(buf, "quit") == 0) {
			printf("\n���� ���� ����");
			exit(0);
		}

		Sleep(1000);
	}
}

unsigned int WINAPI ThreadRecv(void* arg) {
	SOCKET sock = *(SOCKET*)arg;
	char buf[BUFSIZE + 1];
	int retval;
	
	while (1)
	{

		// ������ �ޱ�
		retval = recv(sock, buf, (int)strlen(buf), 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}

		// ���� ������ ���
		buf[retval] = '\0';
		printf("%s", buf);
	}
}

void setName(char* name, int buffer_size, SOCKET sock) {
	int retval;
	int len;
	char check[NAMESIZE+1];

	while (1) {
		printf("\n�г����� �Է��ϼ��� : ");
		if (fgets(name, buffer_size, stdin) == NULL) {
			printf("\n[����] fgets ������� NULL�Դϴ�.");
			break;
		}

		// '\n' ���� ����
		len = (int)strlen(name);
		if (name[len - 1] == '\n')
			name[len - 1] = '\0';
		if (strlen(name) == 0) {
			printf("\n[����] ������ �г������� ������ �� �����ϴ�.");
		}

		retval = send(sock, name, len, 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
		break;

	}
}

int main(int argc, char* argv[])
{
	int retval;
	char name[BUFSIZE + 1];

	// ����� �μ��� ������ IP �ּҷ� ���
	if (argc > 1) SERVERIP = argv[1];

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// ���� ����
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	//�г��� ����
	setName(name, NAMESIZE + 1, sock);

	// ������ ������ ����� ����� �������
	HANDLE recvThread;
	HANDLE sendThread;
	unsigned recvThreadID;
	unsigned sendThreadID;

	//������ ����
	recvThread = (HANDLE)_beginthreadex(NULL, 0, ThreadRecv, &sock, 0, &recvThreadID);
	sendThread = (HANDLE)_beginthreadex(NULL, 0, ThreadSend, &sock, 0, &sendThreadID);

	// main �� ����Ǹ� �� �����嵵 ���� ����ǹǷ� ���� ����ϰ� �Ѵ�
	WaitForSingleObject(recvThread, INFINITE);
	WaitForSingleObject(sendThread, INFINITE);

	// ���� �ݱ�
	closesocket(sock);

	// ���� ����
	WSACleanup();
	return 0;
}
