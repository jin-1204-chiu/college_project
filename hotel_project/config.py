import os

class Config:
    SECRET_KEY = '555'  # app.secret_key
    
    # 資料庫設定
    SERVER = 'CY_OuO'
    DATABASE = 'hotel_system'
    DRIVER = 'ODBC Driver 17 for SQL Server'
    CONNECTION_STRING = f"DRIVER={{{DRIVER}}};SERVER={SERVER};DATABASE={DATABASE};Trusted_Connection=yes;"