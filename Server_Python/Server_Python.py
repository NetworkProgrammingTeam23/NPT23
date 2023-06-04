from socket import *
import time
import threading
import sys
import os


def send_to_all(msg):
    if not clients:  # 클라이언트가 없는 경우
        print("접속자가 없습니다.")
    else:
        for sock in user_list:
            sock.send(msg.encode('euc-kr'))


def off(sock):
    nicknames.remove(user_list[sock][0])
    clients.remove(sock)
    del user_list[sock]


def send_message():
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
    while True:
        try:
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
port = 9900

listeningSock = socket(AF_INET, SOCK_STREAM)
listeningSock.bind((ip, port))
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
