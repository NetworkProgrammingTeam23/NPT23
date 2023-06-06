from socket import *
import time
import threading
import sys
import os


def send_to_all(msg):
    """
    모든 클라이언트에게 메시지를 전송하는 함수입니다.
    :param msg: 전송할 메시지
    """
    if not clients:  # 클라이언트가 없는 경우
        print("접속자가 없습니다.")
    else:
        for sock in user_list:
            #소켓을 통해 데이터를 전송합니다. msg는 전송할 데이터를 나타내며, encode('euc-kr')를 통해 문자열을 지정한 인코딩으로 변환하여 전송합니다.
            sock.send(msg.encode('euc-kr'))


def off(sock):
    """
    클라이언트 접속 종료 시 관련 정보를 삭제하는 함수입니다.
    :param sock: 종료한 클라이언트 소켓
    """
    nicknames.remove(user_list[sock][0])
    clients.remove(sock)
    del user_list[sock]


def send_message():
    """
    서버에서 메시지를 입력받아 모든 클라이언트에게 전송하는 함수입니다.
    """
    while True:
        try:
            msg = input()
            if msg == "/quit":
                global server_running
                server_running = False
                send_to_all("서버가 종료됩니다.")  # 서버 종료 알림 메시지를 전송
                for client in clients:  # 모든 클라이언트 소켓을 닫음
                    client.close()
                listeningSock.close()  # 서버 소켓을 닫음
                sys.exit()
            else:
                send_to_all('Server : {}'.format(msg))
        except ConnectionAbortedError:
            os._exit(0)


def recv_msg(sock):
    """
    클라이언트로부터 메시지를 수신하는 함수입니다.
    :param sock: 수신한 클라이언트 소켓
    """
    while True:
        try:
            #소켓으로부터 데이터를 수신합니다. 1024는 한 번에 수신할 수 있는 최대 데이터 크기를 나타내며, 수신한 데이터는 recvData 변수에 저장됩니다.
            recvData = sock.recv(1024)
            if not recvData:
                break
            recvData = recvData.decode('euc-kr')
            if recvData[0] == '/':
                if recvData[1:] == 'quit':
                    send_to_all('{} 님이 나갔습니다.'.format(user_list[sock][0]))
                    off(sock)
                    sock.close()
                    break
                elif recvData[1:2] == 'w' and len(recvData.split()) > 2:
                    # 귓속말 기능
                    whisper_to = recvData.split()[1]
                    if whisper_to in nicknames:
                        msg = recvData[len(whisper_to) + 4:]
                        sock.send('{} 님에게 : {}'.format(whisper_to, msg).encode('euc-kr'))
                        clients[nicknames.index(whisper_to)].send(
                            '{} 님의 귓속말 : {}'.format(user_list[sock][0], msg).encode('euc-kr'))
                    else:
                        sock.send('닉네임이 존재하지 않습니다.'.encode('euc-kr'))
                elif recvData[1:] == 'member':
                    # 접속자 리스트 출력 기능
                    users = ' '.join(nicknames)
                    sock.send(f'접속자 수 : {len(nicknames)}, 접속자 : {users}'.encode('euc-kr'))
                elif recvData[1:] == 'help':
                    help_msg = '명령어 목록\n/w [닉네임] [메시지]: 지정한 사용자에게만 메시지를 보냅니다.\n' \
                               '/member: 현재 접속 중인 사용자 목록을 볼 수 있습니다.\n' \
                               '/quit: 접속을 종료합니다.'
                    sock.send(help_msg.encode('euc-kr'))
                else:
                    sock.send('잘못된 명령어입니다.'.encode('euc-kr'))
            else:
                msg = '[{}] {} : {}'.format(time.strftime('%X'), user_list[sock][0], recvData)
                send_to_all(msg)
                print(msg)
        except (ConnectionResetError, ConnectionAbortedError):
            print('{}이(가) 나갔습니다.'.format(user_list[sock][0]))
            outid = user_list[sock][0]
            a = clients.index(sock)
            off(sock)
            sock.close()
            if a < len(clients) - 1:
                newsoc = clients[a + 1]
                send_to_all('{}이(가) 나갔습니다.'.format(outid))
            elif a == 0 and len(clients) == 0:
                print("더이상 유저가 없습니다.")
            else:
                newsoc = clients[a - 1]
                send_to_all('{}이(가) 나갔습니다.'.format(outid))
            break


def handle_client(serviceSock, addr):
    """
    클라이언트의 접속을 처리하는 함수입니다.
    :param serviceSock: 클라이언트와의 연결을 담당하는 소켓
    :param addr: 클라이언트의 주소 정보
    """
    print(str(addr), '에서 접속되었습니다.')
    while True:
        try:
            nickname = serviceSock.recv(1024).decode('euc-kr')
            # 닉네임 공백 제거
            nickname = nickname.replace(' ', '')
            if nickname == '':
                # 닉네임을 공백으로만 설정했을 경우
                serviceSock.send('공백'.encode('euc-kr'))
            else:
                if nickname not in nicknames:
                    serviceSock.send('승인'.encode('euc-kr'))
                    break
                else:
                    serviceSock.send('중복'.encode('euc-kr'))
        except ConnectionResetError:
            # 닉네임 설정 전에 접속 종료 시
            serviceSock.close()
            print(str(addr), '닉네임 입력 전에 접속을 종료했습니다.')
            return
    clients.append(serviceSock)
    nicknames.append(nickname)
    user_list[serviceSock] = (nickname, addr)
    send_to_all(nickname + " 님이 접속하였습니다.")
    recv_thread = threading.Thread(target=recv_msg, args=(serviceSock,))
    recv_thread.start()


ip = '127.0.0.1'
port = 9210
# 소켓 생성
listeningSock = socket(AF_INET, SOCK_STREAM)
# 소켓 주소 설정
listeningSock.bind((ip, port))
# 연결 요청 대기 상태 설정
listeningSock.listen(10)
print("접속을 환영합니다.")

send_all = threading.Thread(target=send_message)
send_all.start()

clients = []
nicknames = []
user_list = {}
server_running = True  # 서버가 실행중인지 확인하는 플래그 변수

while server_running:
    try:
        serviceSock, addr = listeningSock.accept()
    except OSError:  # 서버 소켓이 닫히면 OSError가 발생합니다.
        break  # 에러가 발생하면 루프를 빠져나와 서버를 종료합니다.
    client_thread = threading.Thread(target=handle_client, args=(serviceSock, addr))
    client_thread.start()
