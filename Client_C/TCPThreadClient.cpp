#include "Common.h"
#include <process.h>
#include <windows.h>

#define SERVERPORT 9900
#define BUFSIZE    1023
#define NAMESIZE   24
#define RECVSTANDARDDATE 10

char* SERVERIP = (char*)"127.0.0.1";
const char* ACCEPT = "승인";
int namelen;

unsigned int WINAPI ThreadSend(void* arg);
unsigned int WINAPI ThreadRecv(void* arg);
int setName(char* name, int buffer_size, SOCKET sock);
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

	namelen = setName(name, NAMESIZE + 1, sock);
	printf("\n[main] namelen %d", namelen);

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
	printf("\n[send] namelen %d", namelen);
    char buf[BUFSIZE + 1];
    int len;

    while (1) {
        memset(buf, 0, sizeof(buf));		//버퍼 초기화, buf가 0이기에 recv()가 오작동하는 것을 방지
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
			//따라서 recv()를 실행한 버퍼에서 닉네임 뒤의 \0을 지워 나머지 문장도 출력
			if (buf[namelen + RECVSTANDARDDATE] == '\0') {
				buf[namelen + RECVSTANDARDDATE] = ' ';
			}
            buf[retval] = '\0';
            printf("%s\n", buf);
        }
    }
    return 0;
}

//닉네임 설정 함수
int setName(char* name, int buffer_size, SOCKET sock) {
	int retval;
	int len = 0;
	char check[NAMESIZE+1];
	int checklen = sizeof(check);

	while (1) {
		memset(check, 0, checklen);
		printf("\n닉네임을 입력하세요 : ");
		if (fgets(name, buffer_size, stdin) == NULL) {
			printf("\n[오류] fgets의 리턴값이 NULL입니다..");
			break;
		}

		//뒤의 메세지가 안 나오는 현상의 주범인 \0이지만, \n나 \0로 마지막 처리를 안 해주면 오류가 발생
		//(python이나 java는 \0로 print 끝을 구별 안 해서(따로 문자가 있음) 무사히 출력되지만, c는 그렇지 않아서 지금껏 닉네임 뒤의 메세지가 안 나왔음
		len = (int)strlen(name);
		if (name[0]=='\n') {
			printf("\n공백으로 닉네임 설정은 불가능합니다.");
			memset(name, 0, NAMESIZE+1);
			continue;
		}else if (name[len - 1] == '\n')
			name[len - 1] = '\0';

		retval = send(sock, name, len, 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			return 0;
		}
		strcat_s(check, checklen, ACCEPT);
		strcat_s(check, checklen, name);
		printf("\nname %s", name);
		printf("\ncheck %s", check);
		retval = recv(sock, name, NAMESIZE, 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			return 0;
		}
		else if (strcmp(name, check) == 0) {
			printf("\n서버에 입장했습니다.");
			break;
		}

	}

	return len;
}