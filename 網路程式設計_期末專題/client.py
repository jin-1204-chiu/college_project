import tkinter as tk
from PIL import Image, ImageTk
import socket
import threading
import time
import cv2
import os

MAXLINE = 1024
player_index = 0
counts = [-1, -1, -1, -1]
scores = [0, 0, 0, 0]
local_count = 0
video_path = []
video_answers = []
video_questions = []
root = None


def gui_main():
    global counts, scores, player_index, local_count, video_path, video_answers, video_questions, root

    root = tk.Tk()
    root.title('數數比賽')
    root.geometry('800x500')
    root.resizable(False, False)
    
    Label_movie = tk.Label(root)
    Label_movie.place(x=20, y=60, width=650, height=340)

    # 載入規則圖片
    rules_img = Image.open('rule.png').resize((650, 340))
    rules_img_tk = ImageTk.PhotoImage(rules_img)

    # 記住預設圖片
    Label_movie.config(image=rules_img_tk)
    Label_movie.image = rules_img_tk
    
    # 放題目資訊
    canvas_question_bg = tk.Canvas(root, width=800, height=40)
    canvas_question_bg.place(x=0, y=0)
    
    Label_question = tk.Label(canvas_question_bg, text="", font=("Arial", 18))
    Label_question.place(x=50, y=0, width=800, height=40)
    

    # 放玩家資訊
    canvas = tk.Canvas(root, width=800, height=100)
    canvas.place(x=0, y=400)


    def up_plus(event):
        global local_count
        local_count += 1

    def down_minus(event):
        global local_count
        if local_count > 0:
            local_count -= 1

    root.bind("<Up>", up_plus)
    root.bind("<Down>", down_minus)

    # 玩家文字
    player_text_ids = []
    for i in range(4):
        tid = canvas.create_text(10 + 200 * i, 10, text='', anchor='nw', font=('Arial', 20))
        player_text_ids.append(tid)

    def update_text():
        for i in range(4):
            if counts[i] >= 0:
                text = f"玩家 {i+1}: {counts[i]} ({scores[i]})"
            else:
                text = f"玩家 {i+1}: 等待中"
            color = '#0a0' if i == player_index - 1 else '#000' #玩家會看到自己與他人顏色不同
            canvas.itemconfig(player_text_ids[i], text=text, fill=color)
        root.after(20, update_text)

    def play_videos():
        videos = [cv2.VideoCapture(name + '.mp4') for name in video_path]
        video_index = 0

        def score_update():
            global scores
            # 播放完一支影片，根據 counts 與正確答案計算分數
            with threading.Lock():
                for i in range(4):
                    diff = abs(counts[i] - video_answers[video_index])
                    # 計分規則：差距0為3分，差1為2分，差2為1分，差3以上0分
                    if diff == 0:
                        add = 3
                    elif diff == 1:
                        add = 2
                    elif diff == 2:
                        add = 1
                    elif diff <= 3:
                        add = 1
                    else:
                        add = 0
                    scores[i] += add
                print(f"第 {video_index+1} 支影片評分完成，分數：", scores)

        def reset_counts():
            global local_count
            local_count = 0
            try:
                client_socket.sendall(b'reset')
            except:
                pass

        def play_video():
            nonlocal video_index
            if video_index >= len(videos):
                print("所有影片播放完畢")

                # 計算排名
                ranking = sorted([(i + 1, scores[i]) for i in range(4)], key=lambda x: x[1], reverse=True)

                # 顯示排名資訊（彈跳視窗）
                rank_text = "最終排名：\n"
                for idx, (player_id, score) in enumerate(ranking):
                    rank_text += f"第 {idx + 1} 名：玩家 {player_id} - {score} 分\n"

                # 使用 Tkinter 顯示名次
                top = tk.Toplevel()
                top.title("比賽結束！")
                top.geometry("400x200")
                tk.Label(top, text=rank_text, font=("Arial", 16), justify="left").pack(padx=10, pady=10)

                return

            # 還沒播完三部影片
            Label_question.config(text=video_questions[video_index])

            video = videos[video_index]
            if video.isOpened():
                ret, frame = video.read()
                if ret:
                    frame = cv2.resize(frame, (650, 340))
                    frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
                    img = Image.fromarray(frame)
                    img_tk = ImageTk.PhotoImage(image=img)
                    Label_movie.config(image=img_tk)
                    Label_movie.image = img_tk
                    root.after(20, play_video)
                else: # 影片播完
                    video.release()
                    score_update()
                    time.sleep(1)  # 停1秒
                    video_index += 1
                    reset_counts() # 下一題開始count歸零
                    root.after(1000, play_video)
            else:
                print(f"影片 {video_path[video_index]} 播放失敗")
                video_index += 1
                root.after(1000, play_video)

        play_video()

    update_text()

    # 影片或規則圖
    def wait_and_play():
        if len(video_path) == 3 and len(video_questions) == 3:
            # 如果有影片清單了，就開始播放影片（移除規則圖）
            Label_movie.config(image=None)
            play_videos()
        else:
            # 還沒開始，顯示遊戲規則圖片
            Label_movie.config(image=rules_img_tk)
            Label_movie.image = rules_img_tk
            root.after(200, wait_and_play)



    wait_and_play()
    root.mainloop()

def tcp_client():
    global player_index, counts, scores, local_count, video_path, video_answers, client_socket, video_questions
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect(('127.0.0.1', 5678))

    broad_cast = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    broad_cast.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    broad_cast.bind(('', 1234))

    try:
        data = client_socket.recv(1024)
        player_index = int(data.decode('utf-8'))
        if player_index == 0:
            print("伺服器滿人，無法加入")
            return

        def on_player_left(player_left):
            print("on_player_left called for player", player_left)
            top = tk.Toplevel()
            top.title("比賽結束！")
            top.geometry("400x200")
            tk.Label(top, text="player offline", font=("Arial", 16), justify="left").pack(padx=10, pady=10)
            root.after(2000, lambda: os._exit(0))  # 2秒後強制結束程式

        def recv_broadcast():
            global video_path, video_answers, video_questions
            while True:
                data, addr = broad_cast.recvfrom(1024) # 收到影片1,影片2,影片3及答案
                msg = data.decode('utf-8')
                if msg:
                    if msg.startswith("exit:"):
                        player_left = msg.split(":")[1]
                        print("收到離線訊息，準備呼叫 on_player_left")
                        root.after(0, lambda: on_player_left(player_left))
                        break
                    parts = msg.split(',')
                    videos = []
                    answers = []
                    questions_list = []
                    for p in parts:
                        v, a, q = p.split(':', 2)
                        videos.append(v)
                        answers.append(int(a))
                        questions_list.append(q)
                    video_path = videos
                    video_answers = answers
                    video_questions = questions_list

        threading.Thread(target=recv_broadcast, daemon=True).start()

        while True:
            try:
                client_socket.sendall(str(local_count).encode('utf-8'))
                data = client_socket.recv(1024) # 收目前玩家全部計數
                if data:
                    counts_str = data.decode('utf-8')
                    counts = list(map(int, counts_str.split(','))) #字串計數變成list
                time.sleep(0.1)
            except:
                break
    finally:
        client_socket.close()

if __name__ == '__main__':
    threading.Thread(target=tcp_client, daemon=True).start()
    gui_main()