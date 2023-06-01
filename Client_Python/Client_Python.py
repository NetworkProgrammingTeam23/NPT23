from socket import *
import threading
import sys
import os

ip = '127.0.0.1'
port = 9900

clientSock = socket(AF_INET, SOCK_STREAM)
clientSock.connect((ip, port))
client_running = True


def recv_func():
    global client_running
    while True:
        try:
            msg = clientSock.recv(1024).decode('euc-kr')
            if msg == "/quit":  # 서버가 연결 종료 메시지를 보냈는지 확인
                print("서버로부터 연결이 종료되었습니다.")
                client_running = False
                clientSock.close()  # 서버로부터 연결 종료 메시지를 받으면 소켓을 닫습니다.
                os._exit(0)
            print('\n' + msg)
        except (ConnectionAbortedError, ConnectionResetError, OSError):
            print("서버로부터 연결이 종료되었습니다.")
            os._exit(0)  # 프로그램을 종료합니다.


def send_func():
    while True:
        try:
            if not client_running:  # 서버와의 연결이 끊어진 경우
                print("서버와의 연결이 종료되었습니다.")
                os._exit(0)  # 프로그램을 종료합니다.
            else:
                msg = input()
                if msg == '/quit':
                    clientSock.send(msg.encode('euc-kr'))
                    clientSock.close()
                    os._exit(0)
                else:
                    clientSock.send(msg.encode('euc-kr'))
        except OSError:
            print("서버와의 연결이 종료되었습니다.")
            os._exit(0)  # 프로그램을 종료합니다.


while True:
    nickname = input('닉네임을 입력하세요: ')
    clientSock.send(nickname.encode('euc-kr'))
    nickname_possible = clientSock.recv(1024).decode('euc-kr')
    if nickname_possible == '승인':
        print('서버에 입장했습니다.')
        break
    elif nickname_possible == '중복':
        print('중복된 닉네임입니다. 다시 입력해주세요.')

recv_thread = threading.Thread(target=recv_func)
recv_thread.start()

send_thread = threading.Thread(target=send_func)
send_thread.start()
