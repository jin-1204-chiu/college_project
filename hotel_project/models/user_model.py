from .database import Database

class UserModel:
    @staticmethod
    def create_user(user_name, account, password):
        conn = Database.get_connection()
        cursor = conn.cursor()
        try:
            cursor.execute(
                "INSERT INTO Users (user_name, account, password) VALUES (?, ?, ?)",
                (user_name, account, password)
            )
            conn.commit()
            return True
        except Exception as e:
            print("註冊錯誤:", e)
            return False
        finally:
            cursor.close()
            conn.close()

    @staticmethod
    def get_user(account, password=None):
        conn = Database.get_connection()
        cursor = conn.cursor()
        if password:
            cursor.execute("SELECT * FROM Users WHERE account = ? AND password = ?", (account, password))
        else:
            cursor.execute("SELECT * FROM Users WHERE account = ?", (account,))
        
        user = cursor.fetchone()
        cursor.close()
        conn.close()
        return user