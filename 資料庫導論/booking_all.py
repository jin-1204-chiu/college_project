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

# 查詢所有使用者
sql = "SELECT * FROM Booking"
cursor.execute(sql)
bookings = cursor.fetchall()

print("所有使用者：")
for booking in bookings:
    print(f"booking_id: {booking.booking_id}, user_id: {booking.user_id}, room_id: {booking.room_id}, date_start: {booking.date_start}, date_end: {booking.date_end}, pay_type: {booking.pay_type}")

cursor.close()
conn.close()
