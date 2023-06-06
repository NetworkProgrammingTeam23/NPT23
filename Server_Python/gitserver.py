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
    
    # send_to_all(user_list[sock][0]+'님이 나갔습니다.')
    nicknames.remove(user_list[sock][0])
    clients.remove(sock)
    del user_list[sock]

def send_message():
    while True:
        try:
            msg = input()
            if(msg=="quit"):
                global server_running
                server_running = False
                send_to_all("서버가 종료됩니다.")  # 서버 종료 알림 메시지를 전송
                # time.sleep(2)  # 메시지를 보낸 후에 2초 대기
                for client in clients:  # 모든 클라이언트 소켓을 닫음
                    client.close()
                listeningSock.close()  # 서버 소켓을 닫음
                sys.exit()
            else:
                send_to_all(msg)
        except ConnectionAbortedError:
            os._exit(0)
# def outid_message(outid):
#     send_to_all('{}이(가) 나갔습니다.'.format(outid))


def recv_msg(sock):
    while True:
        try:
            recvData = sock.recv(1024)
            if not recvData:
                break
            msg = '[{}] {} : {}'.format(time.strftime('%X'), user_list[sock][0], recvData.decode('euc-kr'))
            send_to_all(msg)
            print(msg)
        except (ConnectionResetError,ConnectionAbortedError):
            print('{}이(가) 나갔습니다.'.format(user_list[sock][0]))
            outid = user_list[sock][0]
            a=clients.index(sock)
            off(sock)
            sock.close()
            if a < len(clients) - 1:
                newsoc = clients[a+1]
                send_to_all('{}이(가) 나갔습니다.'.format(outid))
            elif(a==0 and len(clients)==0):
                print("더이상 유저가 없습니다.")
            else :
                newsoc = clients[a-1]
                send_to_all('{}이(가) 나갔습니다.'.format(outid))
            break
        # except ConnectionAbortedError:
        #     # ConnectionAbortedError 예외 처리 추가
        #     print("서버가 종료되었습니다.")
        #     break
def handle_client(serviceSock, addr):
    print(str(addr), '에서 접속되었습니다.')
    while True:
        nickname = serviceSock.recv(1024).decode('euc-kr')
        if nickname not in nicknames:
            serviceSock.send('승인'.encode('euc-kr'))
            break
        else:
            serviceSock.send('중복'.encode('euc-kr'))
    clients.append(serviceSock)
    nicknames.append(nickname)
    user_list[serviceSock] = (nickname, addr)
    send_to_all(nickname+"님이 접속하였습니다.")
    recv_thread = threading.Thread(target=recv_msg, args=(serviceSock,))
    recv_thread.start()
    
ip = '127.0.0.1'
port = 9900

listeningSock = socket(AF_INET, SOCK_STREAM)
listeningSock.bind((ip, port))
listeningSock.listen(10)
print("접속을 환영합니다.")

send_all=threading.Thread(target=send_message)
send_all.start()

clients=[]
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