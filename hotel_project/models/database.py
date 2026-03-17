import pyodbc
from config import Config

class Database:
    @staticmethod
    def get_connection():
        return pyodbc.connect(Config.CONNECTION_STRING)