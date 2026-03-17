from .database import Database
from datetime import datetime

class BookingModel:
    @staticmethod
    def get_by_user(user_id):
        conn = Database.get_connection()
        cursor = conn.cursor()
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
        return bookings

    @staticmethod
    def create(user_id, room_id, date_start, date_end, pay_type):
        conn = Database.get_connection()
        cursor = conn.cursor()
        try:
            cursor.execute('''
                INSERT INTO Booking (user_id, room_id, date_start, date_end, pay_type)
                VALUES (?, ?, ?, ?, ?)
            ''', (user_id, room_id, date_start, date_end, pay_type))
            conn.commit()
            return True
        except Exception as e:
            print(f"訂房錯誤: {e}")
            return False
        finally:
            cursor.close()
            conn.close()

    @staticmethod
    def cancel(booking_id):
        conn = Database.get_connection()
        cursor = conn.cursor()
        cursor.execute("DELETE FROM Booking WHERE booking_id = ?", (booking_id,))
        conn.commit()
        success = cursor.rowcount > 0
        cursor.close()
        conn.close()
        return success

    @staticmethod
    def update(booking_id, room_id, date_start, date_end, pay_type):
        conn = Database.get_connection()
        cursor = conn.cursor()
        
        # 檢查衝突 (排除自己這筆訂單)
        cursor.execute('''
            SELECT COUNT(*) FROM Booking
            WHERE room_id = ? AND booking_id != ? AND NOT (date_end < ? OR date_start > ?)
        ''', (room_id, booking_id, date_start, date_end))
        
        if cursor.fetchone()[0] > 0:
            conn.close()
            return False

        cursor.execute('''
            UPDATE Booking
            SET date_start = ?, date_end = ?, pay_type = ?
            WHERE booking_id = ?
        ''', (date_start, date_end, pay_type, booking_id))
        
        conn.commit()
        conn.close()
        return True