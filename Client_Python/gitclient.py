from socket import *
import threading

ip = '127.0.0.1'
port = 9900

clientSock = socket(AF_INET, SOCK_STREAM)
clientSock.connect((ip, port))

def recv_func():
    while True:
        try:
            msg = clientSock.recv(1024).decode('euc-kr')
            print('\n'+msg)
        except ConnectionAbortedError:
            clientSock.close()
            break
def send_func():
    while True:
        msg = input()
        if msg == 'quit':
            clientSock.send(msg.encode('euc-kr'))
            clientSock.close()
            break
        else:
            clientSock.send(msg.encode('euc-kr'))

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