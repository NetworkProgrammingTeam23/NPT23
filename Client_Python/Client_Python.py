from socket import *
import threading
import sys
import os
import time
ip = '127.0.0.1'
port = 9210

clientSock = socket(AF_INET, SOCK_STREAM)
clientSock.connect((ip, port))
client_running = True
send_time = 0
received_time=0
def recv_func():
    global client_running
    global received_time
    while True:
        try:
            msg = clientSock.recv(1024).decode('euc-kr')
            if msg == "/quit":  # 서버가 연결 종료 메시지를 보냈는지 확인
                print("서버로부터 연결이 종료되었습니다.")
                client_running = False
                clientSock.close()  # 서버로부터 연결 종료 메시지를 받으면 소켓을 닫습니다.
                os._exit(0)
            received_time = time.perf_counter()
            elapsed_time = received_time-send_time
            print("메시지 수신 시간: ", elapsed_time, "초")    
            print('\n' + msg)
        except (ConnectionAbortedError, ConnectionResetError, OSError):
            print("서버로부터 연결이 종료되었습니다.")
            os._exit(0)  # 프로그램을 종료합니다.

def send_func():
    
    while True:
        global send_time
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
                    send_time = time.perf_counter()
                    clientSock.send(msg.encode('euc-kr'))
        except OSError:
            print("서버와의 연결이 종료되었습니다.")
            os._exit(0)  # 프로그램을 종료합니다.


while True:
    nickname = input('닉네임을 입력하세요: ')
    if not nickname:
        # 입력 없이 엔터키 쳤을 때
        continue
    clientSock.send(nickname.encode('euc-kr'))
    nickname_possible = clientSock.recv(1024).decode('euc-kr')
    if nickname_possible == '공백':
        # 닉네임을 공백으로만 설정했을 경우
        print('닉네임을 공백만으로 설정할 수 없습니다.')
    else:
        if nickname_possible == '승인':
            print('서버에 입장했습니다. 명령어는 /help로 확인하세요.')
            break
        elif nickname_possible == '중복':
            print('중복된 닉네임입니다. 다시 입력해주세요.')

recv_thread = threading.Thread(target=recv_func)
recv_thread.start()

send_thread = threading.Thread(target=send_func)
send_thread.start()
