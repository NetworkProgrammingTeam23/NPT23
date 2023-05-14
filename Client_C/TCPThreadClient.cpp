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

	//통신을 담당하는 스레드 생성
	recvThread = (HANDLE)_beginthreadex(NULL, 0, ThreadRecv, &sock, 0, &recvThreadID);
	sendThread = (HANDLE)_beginthreadex(NULL, 0, ThreadSend, &sock, 0, &sendThreadID);

	//main스레드 무한 대기, 메인이 종료되면 나머지 스레드도 종료됨.
	WaitForSingleObject(recvThread, INFINITE);
	WaitForSingleObject(sendThread, INFINITE);

	closesocket(sock);

	WSACleanup();
	return 0;
}

//send() 스레드
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
			printf("\n서버와의 연결이 종료되었습니다");
			exit(0);		//quit으로 신호를 줘서 recv 스레드를 종료하는 방식은 느려서 프로세스를 종료시키는 방향으로 갔습니다.
		}
	}
}

//recv() 스레드
unsigned int WINAPI ThreadRecv(void* arg) {
    SOCKET sock = *(SOCKET*)arg;
    char buf[BUFSIZE + 1];
    int len;

    while (1) {
        memset(buf, 0, sizeof(buf));		//해당 과정을 추가해서 한 줄로 나타내는 데에는 성공했습니다.
        int retval = recv(sock, buf, BUFSIZE, 0);
        if (retval == SOCKET_ERROR) {
            err_display("recv()");
            break;
        }
        else if (retval == 0) {
            printf("\n서버로부터 연결이 종료되었습니다.");
            break;
        }
        else {
            buf[retval] = '\0';
            printf("%s\n", buf);		//msg = '[{}] {} : {}'.format(time.strftime('%X'), user_list[sock][0], recvData.decode('euc-kr'))에서 recvData.decode('euc-kr')만 못받음
        }
    }
    return 0;
}

//닉네임 설정 함수
void setName(char* name, int buffer_size, SOCKET sock) {
	int retval;
	int len;

	while (1) {
		printf("\n닉네임을 입력하세요 : ");
		if (fgets(name, buffer_size, stdin) == NULL) {
			printf("\n[오류] fgets의 리턴값이 NULL입니다..");
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

		//아래 if문이 안 먹혀서 임시로 두었습니다. 현재 확인문이 승인닉네임으로 나오는 건지 뒤에 빈 공간도 무언가 있는 건지 알 수 없습니다.
		break;
		if (strcmp(name, "승인") == 0) {
			printf("서버에 입장했습니다.");
			break;
		}

	}
}

//안 통했지만, 일단 두었습니다.
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