#include "Common.h"
#include <windows.h>
#include <process.h>

char* SERVERIP = (char*)"127.0.0.1";
#define SERVERPORT 9360
#define BUFSIZE    512
#define NAMESIZE   16

int retval;

unsigned int WINAPI ThreadSend(void* arg) {
	SOCKET sock = *((SOCKET**)arg)[0];
	char* name = ((char**)arg)[1];
	int len;
	char buf[BUFSIZE + 1];

	while (1) {
		// 데이터 입력
		printf("\n[보낼 데이터] ");
		if (fgets(buf, BUFSIZE + 1, stdin) == NULL)
			break;

		len = (int)strlen(buf);
		if (buf[len - 1] == '\n')
			buf[len - 1] = '\0';
		if (strlen(buf) == 0)
			break;

		// 데이터 보내기
		retval = send(sock, buf, (int)strlen(buf), 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
		printf("[%s] %s\n", name, buf);
	}
}

unsigned int WINAPI ThreadRecv(void* arg) {
	SOCKET sock = *((SOCKET**)arg)[0];
	char* name = ((char**)arg)[1];
	char buf[BUFSIZE + 1];
	
	while (1)
	{
		// 데이터 받기
		retval = recv(sock, buf, retval, MSG_WAITALL);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		// 받은 데이터 출력
		buf[retval] = '\0';
		printf("[받은 데이터] %s\n", buf);
	}
}

void setName(char* name, int buffer_size, SOCKET sock) {
	int retval;
	int len;
	char check[1];

	while (1) {
		printf("\n닉네임을 입력하세요 : ");
		if (fgets(name, buffer_size, stdin) != NULL) {
			printf("\n[오류] fgets 결과값이 NULL입니다.");
			break;
		}

		// '\n' 문자 제거
		len = (int)strlen(name);
		if (name[len - 1] == '\n')
			name[len - 1] = '\0';
		if (strlen(name) == 0) {
			printf("\n[오류] 공백을 닉네임으로 설정할 수 없습니다.");
		}

		retval = send(sock, name, (int)strlen(name), 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
		retval = recv(sock, check, buffer_size, 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (check[0] == 'n')
			printf("\n[중복되는 닉네임]");
		else {
			break;
		}

	}
}

int main(int argc, char* argv[])
{
	int retval;
	char name[BUFSIZE + 1];

	// 명령행 인수가 있으면 IP 주소로 사용
	if (argc > 1) SERVERIP = argv[1];

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 소켓 생성
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

	//닉네임 설정
	setName(name, NAMESIZE + 1, sock);

	// 서버와 데이터 통신을 담당할 스레드들
	HANDLE recvThread;
	HANDLE sendThread;
	unsigned recvThreadID;
	unsigned sendThreadID;
	void* socketAndName[2];
	socketAndName[0] = &sock;
	socketAndName[1] = name;

	//스레드 생성
	recvThread = (HANDLE)_beginthreadex(NULL, 0, ThreadRecv, &socketAndName, 0, &recvThreadID);
	sendThread = (HANDLE)_beginthreadex(NULL, 0, ThreadSend, &socketAndName, 0, &sendThreadID);

	// main 이 종료되면 두 스레드도 강제 종료되므로 무한 대기하게 한다
	WaitForSingleObject(recvThread, INFINITE);
	WaitForSingleObject(sendThread, INFINITE);

	// 소켓 닫기
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}
