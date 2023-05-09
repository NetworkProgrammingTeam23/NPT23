from socket import *
import time
import threading
import sys
import asyncio
def send_to_all(msg):
    for sock in user_list:
        sock.send(msg.encode('euc-kr'))
        
def off(sock):
    
    # send_to_all(user_list[sock][0]+'님이 나갔습니다.')
    nicknames.remove(user_list[sock][0])
    clients.remove(sock)
    del user_list[sock]

def send_message():
    while True:
        msg = input()
        if(msg=="quit"):
            listeningSock.close()
            sys.exit()
        else:
            listeningSock.send_to_all(msg.encode('euc-kr'))

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
        except ConnectionResetError:
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
while True:
    serviceSock, addr = listeningSock.accept()
    print(str(addr), '에서 접속되었습니다.')
    while True:
        nickname = serviceSock.recv(1024).decode('euc-kr')
        if nickname not in nicknames:
            serviceSock.send('승인'.encode('euc-kr'))
            break
        else:
            serviceSock.send('중복'.encode('euc-kr'))
    send_to_all(nickname+"님이 접속하였습니다.")
    
    clients.append(serviceSock)
    nicknames.append(nickname)
    user_list[serviceSock] = (nickname, addr)

    recv_thread = threading.Thread(target=recv_msg, args=(serviceSock,))
    recv_thread.start()