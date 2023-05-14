#include "Common.h"
#include <process.h>
#include <windows.h>

#define SERVERPORT 9900
#define BUFSIZE    1023
#define NAMESIZE   1023

char* SERVERIP = (char*)"127.0.0.1";
char printBuffer[BUFSIZE+1];

unsigned int WINAPI ThreadSend(void* arg);
unsigned int WINAPI ThreadRecv(void* arg);
void setName(char* name, int buffer_size, SOCKET sock);
int recvn(SOCKET sock, char* buf, int buflen);


int main(int argc, char* argv[])
{
	int retval;
	char name[BUFSIZE + 1];

	if (argc > 1) SERVERIP = argv[1];

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

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

	setName(name, NAMESIZE + 1, sock);

	HANDLE recvThread;
	HANDLE sendThread;
	unsigned recvThreadID;
	unsigned sendThreadID;

	//����� ����ϴ� ������ ����
	recvThread = (HANDLE)_beginthreadex(NULL, 0, ThreadRecv, &sock, 0, &recvThreadID);
	sendThread = (HANDLE)_beginthreadex(NULL, 0, ThreadSend, &sock, 0, &sendThreadID);

	//main������ ���� ���, ������ ����Ǹ� ������ �����嵵 �����.
	WaitForSingleObject(recvThread, INFINITE);
	WaitForSingleObject(sendThread, INFINITE);

	closesocket(sock);

	WSACleanup();
	return 0;
}

//send() ������
unsigned int WINAPI ThreadSend(void* arg) {
	SOCKET sock = *(SOCKET*)arg;
	int len;
	char buf[BUFSIZE + 1];
	int retval;

	while (1) {

		if (fgets(buf, BUFSIZE + 1, stdin) == NULL)
			break;

		len = (int)strlen(buf);
		if (buf[len - 1] == '\n')
			buf[len - 1] = '\0';

		retval = send(sock, buf, (int)strlen(buf), 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
		else if (strcmp(buf, "quit") == 0) {
			printf("\n�������� ������ ����Ǿ����ϴ�");
			exit(0);		//quit���� ��ȣ�� �༭ recv �����带 �����ϴ� ����� ������ ���μ����� �����Ű�� �������� �����ϴ�.
		}
	}
}

//recv() ������
unsigned int WINAPI ThreadRecv(void* arg) {
    SOCKET sock = *(SOCKET*)arg;
    char buf[BUFSIZE + 1];
    int len;

    while (1) {
        memset(buf, 0, sizeof(buf));		//�ش� ������ �߰��ؼ� �� �ٷ� ��Ÿ���� ������ �����߽��ϴ�.
        int retval = recv(sock, buf, BUFSIZE, 0);
        if (retval == SOCKET_ERROR) {
            err_display("recv()");
            break;
        }
        else if (retval == 0) {
            printf("\n�����κ��� ������ ����Ǿ����ϴ�.");
            break;
        }
        else {
            buf[retval] = '\0';
            printf("%s\n", buf);		//msg = '[{}] {} : {}'.format(time.strftime('%X'), user_list[sock][0], recvData.decode('euc-kr'))���� recvData.decode('euc-kr')�� ������
        }
    }
    return 0;
}

//�г��� ���� �Լ�
void setName(char* name, int buffer_size, SOCKET sock) {
	int retval;
	int len;

	while (1) {
		printf("\n�г����� �Է��ϼ��� : ");
		if (fgets(name, buffer_size, stdin) == NULL) {
			printf("\n[����] fgets�� ���ϰ��� NULL�Դϴ�..");
			break;
		}

		len = (int)strlen(name);
		if (name[len - 1] == '\n')
			name[len - 1] = '\0';

		retval = send(sock, name, len, 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}

		//�Ʒ� if���� �� ������ �ӽ÷� �ξ����ϴ�. ���� Ȯ�ι��� ���δг������� ������ ���� �ڿ� �� ������ ���� �ִ� ���� �� �� �����ϴ�.
		break;
		if (strcmp(name, "����") == 0) {
			printf("������ �����߽��ϴ�.");
			break;
		}

	}
}

//�� ��������, �ϴ� �ξ����ϴ�.
int recvn(SOCKET sock, char* buf, int buflen) {
	int totalLen = 0;
	while (totalLen < buflen) {
		int recvLen = recv(sock, buf + totalLen, buflen - totalLen, 0);
		if (recvLen <= 0)
			return recvLen;
		totalLen += recvLen;
	}
	return totalLen;
}