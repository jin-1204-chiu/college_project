import socket
import threading
import random
import time

MAX_CLIENT = 4
clients = [None] * MAX_CLIENT
counts = [-1] * MAX_CLIENT
scores = [0] * MAX_CLIENT
lock = threading.Lock()

video_name = {
    'dance': 14,
    'vapo': 2,
    'ok': 4,
    'ghost': 23,
    'mushroom': 24,
    'boom': 18
}

questions = {
    'dance': "請計數吉伊身邊有幾隻其他的小動物",
    'vapo': "請計數影片中說vapo的次數",
    'ok': "請計數美樂蒂貼錯幾次ok蹦",
    'ghost': "請計數有幾個幽靈(有時會變透明)",
    'mushroom': "請計數蘑菇人的數量",
    'boom': "請數有幾個炸彈(只看最後剩下的)"
}

def client_thread(sock, clnt_index):
    global counts, scores, broad_cast
    try:
        while True:
            data = sock.recv(1024) # 接收count數量
            if not data:
                break
            msg = data.decode('utf-8')

            with lock:
                if msg == 'reset': #下一題時計數歸零
                    counts[clnt_index] = 0
                else:
                    try:
                        count = int(msg) # 接收每個玩家計數
                        counts[clnt_index] = count
                    except:
                        pass
            
            with lock:
                all_counts_str = ','.join(str(c) for c in counts)
            sock.sendall(all_counts_str.encode('utf-8')) #TCP傳全部玩家計數資訊給client
    except Exception as e:
        print(f"Exception from client[{clnt_index}]: {e}")
    finally:
        with lock:
            clients[clnt_index] = None
            counts[clnt_index] = -1
            scores[clnt_index] = 0
            # 廣播通知所有 client 有人離線，結束遊戲
            exit_msg = f"exit:{clnt_index + 1}"
            broad_cast.sendto(exit_msg.encode('utf-8'), ('<broadcast>', 1234))
        sock.close()
        print(f"Client {clnt_index + 1} disconnected")

#廣播
broad_cast = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) #UDP
broad_cast.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)

def main():
    global clients, scores
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM) #TCP
    server_socket.bind(('0.0.0.0', 5678))
    server_socket.listen(MAX_CLIENT)
    print("Server is running, waiting for clients...")

    while True:
        client_sock, addr = server_socket.accept()
        with lock:
            for i in range(MAX_CLIENT): 
                if clients[i] is None:
                    clients[i] = client_sock 
                    scores[i] = 0  # 新玩家分數歸零
                    client_sock.sendall(str(i + 1).encode('utf-8')) #傳有player_id
                    threading.Thread(target=client_thread, args=(client_sock, i), daemon=True).start()
                    print(f"New client connected as player {i + 1}")
                    break
            else:
                client_sock.sendall(b'0') # 超過可玩玩家數量
                client_sock.close()

        # 等待4個玩家都連線後，開始廣播影片清單，並重選影片
        with lock:
            if all(c is not None for c in clients):
                selected = random.sample(list(video_name.keys()), 3) #不會選到重覆影片
                broadcast_msg = ','.join(f"{v}:{video_name[v]}:{questions[v]}" for v in selected)
                print("開始廣播影片:", broadcast_msg)
                for _ in range(10): # 傳多次，因為擔心UDP沒有傳送到client到client
                    # 傳此回合影片名稱(共三個)
                    broad_cast.sendto(broadcast_msg.encode('utf-8'), ('<broadcast>', 1234))
                    time.sleep(0.3)
        

if __name__ == "__main__":
    main()