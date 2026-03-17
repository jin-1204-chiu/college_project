import pyodbc

# 資料庫連線設定
server = 'GIGABYTE'
database = 'hotel_system'
driver = 'ODBC Driver 17 for SQL Server'

connection_string = (
    f"DRIVER={{{driver}}};"
    f"SERVER={server};"
    f"DATABASE={database};"
    f"Trusted_Connection=yes;"
)

# 連接資料庫
conn = pyodbc.connect(connection_string)
cursor = conn.cursor()

# 查詢所有房間資料
sql = "SELECT * FROM Room"
cursor.execute(sql)
rooms = cursor.fetchall()

print("所有房間資料：")
for room in rooms:
    # 根據你的資料表欄位，調整印出欄位名稱
    print(f"room_id: {room.room_id}, room_type: {room.room_type}, room_description: {room.room_description}, room_image: {room.room_image}, room_price: {room.room_price}")

cursor.close()
conn.close()
