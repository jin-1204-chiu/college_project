from .database import Database

class RoomModel:
    @staticmethod
    def search_available(date_start, date_end, room_type=None):
        conn = Database.get_connection()
        cursor = conn.cursor()
        
        sql = '''
            SELECT * FROM Room
            WHERE room_id NOT IN (
                SELECT room_id FROM Booking
                WHERE NOT (date_end < ? OR date_start > ?)
            )
        '''
        params = [date_start, date_end]

        if room_type:
            sql += " AND room_type = ?"
            params.append(room_type)
        
        cursor.execute(sql, params)
        rooms = cursor.fetchall()
        cursor.close()
        conn.close()
        return rooms