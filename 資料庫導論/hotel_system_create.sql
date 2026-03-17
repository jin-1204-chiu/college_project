IF NOT EXISTS (SELECT * FROM sys.databases WHERE name = 'hotel_system')
BEGIN
    CREATE DATABASE hotel_system;
END;
GO

USE hotel_system;
GO
