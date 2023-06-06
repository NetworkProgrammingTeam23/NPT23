#include "Common.h"
#include <process.h>
#include <windows.h>

#define SERVERPORT 9210
#define BUFSIZE    1023
#define NAMESIZE   1024
#define RECVSTANDARDDATE 10

char* SERVERIP = (char*)"127.0.0.1";	//���� �ּ�
const char ACCEPT[] = "����";			//���� ���� ���� ��ȣ
int namelen;							//�г��� ����

unsigned int WINAPI ThreadSend(void* arg);					//send() ������
unsigned int WINAPI ThreadRecv(void* arg);					//recv() ������
int setName(char* name, int buffer_size, SOCKET sock);		//�г��� ����


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

	namelen = setName(name, NAMESIZE + 1, sock);

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
		else if (strcmp(buf, "/quit") == 0) {
			printf("\n�������� ������ ����Ǿ����ϴ�\n");
			exit(0);		//quit���� ��ȣ�� �༭ recv �����带 �����ϴ� ����� ������ ���μ����� �����Ű�� �������� �����ϴ�.
		}
	}
}

//recv() ������
unsigned int WINAPI ThreadRecv(void* arg) {
	SOCKET sock = *(SOCKET*)arg;
    char buf[BUFSIZE + 1];

    while (1) {
        memset(buf, 0, sizeof(buf));		//���� �ʱ�ȭ, buf�� 0�̱⿡ recv()�� ���۵��ϴ� ���� ����
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
			//���� recv()�� ������ ���ۿ��� �г��� ���� \0�� ���� ������ ���嵵 ���
			if (buf[namelen + RECVSTANDARDDATE] == '\0') {
				buf[namelen + RECVSTANDARDDATE] = ' ';
			}
            buf[retval] = '\0';
            printf("%s\n", buf);
        }
    }
    return 0;
}

//�г��� ���� �Լ�
int setName(char* name, int buffer_size, SOCKET sock) {
	int retval;
	int len = 0;
	char check[NAMESIZE+1];
	int checklen = sizeof(check);
	char frashBuffer;

	while (1) {
		memset(name, 0, sizeof(name));
		memset(check, 0, checklen);
		printf("\n�г����� �Է��ϼ��� : ");
		if (fgets(name, buffer_size, stdin) == NULL) {
			printf("\n[����] fgets�� ���ϰ��� NULL�Դϴ�..");
			break;
		}

		//���� �޼����� �� ������ ������ �ֹ��� \0������, \n�� \0�� ������ ó���� �� ���ָ� ������ �߻�
		//(python�̳� java�� \0�� print ���� ���� �� �ؼ�(���� ���ڰ� ����) ������ ��µ�����, c�� �׷��� �ʾƼ� ���ݲ� �г��� ���� �޼����� �� ������
		len = (int)strlen(name);
		if (len == 1) {
			printf("\n�������� �г��� ������ �Ұ����մϴ�.");
		}
		else if (name[len - 1] == '\n') {
			printf("\n\0����");
			name[len - 1] = '\0';
			len = (int)strlen(name);

			retval = send(sock, name, len, 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				return 0;
			}

			retval = recv(sock, check, NAMESIZE, 0);
		
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
				return 0;
			}
			else if (strcmp(check, ACCEPT) == 0) {
				printf("\n������ �����߽��ϴ�.\n");
				break;
			}
			else if (strcmp(check, "�ߺ�") == 0) {
				printf("�ߺ��� �г����Դϴ�. �ٽ� �Է����ּ���.\n");
			}
		}

	}

	return len;
}