from flask import Flask, render_template, request, redirect, url_for, session, jsonify
import pyodbc
from datetime import date
from datetime import datetime


app = Flask(__name__, template_folder='web_html')  # 這裡指定模板資料夾改成 web_html
app.secret_key = '555'  # 可以隨便設，但不要留空


# 資料庫連線設定
def get_db_connection():
    server = 'GIGABYTE'
    database = 'hotel_system'
    driver = 'ODBC Driver 17 for SQL Server'
    conn_str = f"DRIVER={{{driver}}};SERVER={server};DATABASE={database};Trusted_Connection=yes;"
    conn = pyodbc.connect(conn_str)
    return conn

# 根目錄
@app.route('/')
def index():
    return redirect(url_for('hotel_homePage'))

# 首頁資訊
@app.route('/index', methods=['GET', 'POST'])
def hotel_homePage():
    return render_template('index.html')

# 註冊介面
@app.route('/register', methods=['POST'])
def register():
    user_name = request.form['user_name']
    account = request.form['account']
    password = request.form['password'] 

    conn = get_db_connection()
    cursor = conn.cursor()

    try:
        cursor.execute(
            "INSERT INTO Users (user_name, account, password) VALUES (?, ?, ?)",
            (user_name, account, password)
        )

        conn.commit()
        return "success"
    except Exception as e:
        print("註冊錯誤:", e)
        return "fail"
    finally:
        cursor.close()
        conn.close()

# 登入介面
@app.route('/login', methods=['POST'])
def login():
    account = request.form['account']
    password = request.form['password']

    conn = get_db_connection()
    cursor = conn.cursor()

    cursor.execute("SELECT * FROM Users WHERE account = ? AND password = ?", (account, password))
    user = cursor.fetchone()

    cursor.close()
    conn.close()

    if user:
        session['user_name'] = user.user_name
        session['account'] = user.account
        return jsonify({'status': 'success'})
    else:
        return jsonify({'status': 'fail'})

# 登出介面
@app.route('/logout')
def logout():
    session.clear()
    return redirect(url_for('hotel_homePage'))

# 購物車功能
@app.route('/shopping_list')
def shopping_list():
    if 'account' not in session:
        return redirect(url_for('hotel_homePage'))

    conn = get_db_connection()
    cursor = conn.cursor()

    cursor.execute("SELECT user_id FROM Users WHERE account = ?", (session['account'],))
    user = cursor.fetchone()

    bookings = []
    if user:
        user_id = user.user_id
        cursor.execute('''
            SELECT B.booking_id, B.date_start, B.date_end, B.pay_type,
                   R.room_type, R.room_price, R.room_description, R.room_id
            FROM Booking B
            JOIN Room R ON B.room_id = R.room_id
            WHERE B.user_id = ?
        ''', (user_id,))
        bookings = cursor.fetchall()

    cursor.close()
    conn.close()

    return render_template('shopping_list.html', bookings=bookings, user=session['user_name'])


# 刪除購物車內容
@app.route('/cancel_booking', methods=['POST'])
def cancel_booking():
    data = request.get_json()
    booking_id = data.get('booking_id')

    if not booking_id:
        return jsonify({'status': 'fail', 'message': '缺少 booking_id'})

    conn = get_db_connection()
    cursor = conn.cursor()

    try:
        cursor.execute("DELETE FROM Booking WHERE booking_id = ?", (booking_id,))
        conn.commit()
        result = cursor.rowcount
        cursor.close()
        conn.close()

        if result > 0:
            return jsonify({'status': 'success'})
        else:
            return jsonify({'status': 'fail', 'message': '找不到該筆訂單'})

    except Exception as e:
        return jsonify({'status': 'fail', 'message': str(e)})

# 編輯購物車內容
@app.route('/edit_booking', methods=['POST'])
def edit_booking():
    booking_id = request.form['booking_id']
    room_id = request.form['room_id']
    date_start = request.form['date_start']
    date_end = request.form['date_end']
    pay_type = request.form['pay_type']

    start = datetime.strptime(date_start, '%Y-%m-%d')
    end = datetime.strptime(date_end, '%Y-%m-%d') 
    
    if end <= start:
        return "<script>alert('退房日必須晚於入住日'); window.history.back();</script>"

    conn = get_db_connection()
    cursor = conn.cursor()

    # 確認新日期沒有和別的訂單重疊
    cursor.execute('''
        SELECT COUNT(*) FROM Booking
        WHERE room_id = ? AND booking_id != ? AND NOT (date_end < ? OR date_start > ?)
    ''', (room_id, booking_id, date_start, date_end))

    conflict_count = cursor.fetchone()[0]

    if conflict_count > 0:
        conn.close()
        return "<script>alert('此日期已被預訂，請選擇其他時間'); window.history.back();</script>"

    # 更新訂單
    cursor.execute('''
        UPDATE Booking
        SET date_start = ?, date_end = ?, pay_type = ?
        WHERE booking_id = ?
    ''', (date_start, date_end, pay_type, booking_id))

    conn.commit()
    conn.close()

    return redirect(url_for('shopping_list'))


# 查詢空房頁面
@app.route('/search', methods=['GET', 'POST'])
def search_rooms():
    rooms = []
    date_start = ''
    date_end = ''
    room_type = ''

    room_type = request.args.get('room_type') or request.form.get('room_type')
    today = date.today().strftime('%Y-%m-%d')

    if request.method == 'POST':
        date_start = request.form['date_start']
        date_end = request.form['date_end']
        room_type = request.form['room_type']
        
        conn = get_db_connection()
        cursor = conn.cursor()
        sql = '''
            SELECT * FROM Room
            WHERE room_id NOT IN (
                SELECT room_id FROM Booking
                WHERE NOT (date_end < ? OR date_start > ?)
            )
        '''
        params = [date_start, date_end]

        # 如果有選房型，就加條件
        if room_type:
            sql += " AND room_type = ?"
            params.append(room_type)
        
        cursor.execute(sql, params)
        rooms = cursor.fetchall()
        cursor.close()
        conn.close()
    return render_template('search_room.html', rooms=rooms, date_start=date_start, date_end=date_end, room_type = room_type, current_date=today)


# 新增訂房頁面
@app.route('/booking', methods=['GET', 'POST'])
def booking():
    message = ''
    if request.method == 'POST':
        user_id = request.form['user_id']
        room_id = request.form['room_id']
        date_start = request.form['date_start']
        date_end = request.form['date_end']
        pay_type = request.form['pay_type']
        
        conn = get_db_connection()
        cursor = conn.cursor()
        try:
            cursor.execute('''
                INSERT INTO Booking (user_id, room_id, date_start, date_end, pay_type)
                VALUES (?, ?, ?, ?, ?)
            ''', (user_id, room_id, date_start, date_end, pay_type))
            conn.commit()
            message = '訂房成功！'
        except Exception as e:
            message = f'訂房失敗: {e}'
        finally:
            cursor.close()
            conn.close()
    return render_template('booking_room.html', message=message)


@app.route('/booking_confirm', methods=['POST'])
def booking_confirm():
    if 'user_name' not in session:
        return 'unauthorized'

    room_id = request.form['room_id']
    date_start = request.form['date_start']
    date_end = request.form['date_end']
    pay_type = request.form['payment']
    account = session['account']

    try:
        conn = get_db_connection()
        cursor = conn.cursor()

        cursor.execute("SELECT user_id FROM Users WHERE account = ?", (account,))
        user = cursor.fetchone()

        if user:
            user_id = user.user_id
            cursor.execute('''
                INSERT INTO Booking (user_id, room_id, date_start, date_end, pay_type)
                VALUES (?, ?, ?, ?, ?)
            ''', (user_id, room_id, date_start, date_end, pay_type))
            conn.commit()
            result = 'success'
        else:
            result = 'fail'
        
    except Exception as e:
        print("訂房錯誤：", e)
        result = 'fail'
    
    finally:
        cursor.close()
        conn.close()

    return result




if __name__ == '__main__':
    app.run(debug=True)
