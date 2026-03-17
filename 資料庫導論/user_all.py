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
sql = "SELECT * FROM Users"
cursor.execute(sql)
users = cursor.fetchall()

print("所有使用者：")
for user in users:
    print(f"user_id: {user.user_id}, user_name: {user.user_name}, account: {user.account}, password: {user.password}")

cursor.close()
conn.close()
