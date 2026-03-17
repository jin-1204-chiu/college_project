USE hotel_system;
GO

-- 建立使用者表
CREATE TABLE Users (
    user_id INT IDENTITY(1,1) PRIMARY KEY,
    user_name NVARCHAR(100) NOT NULL,
    account NVARCHAR(100) NOT NULL UNIQUE,
    password NVARCHAR(100) NOT NULL
);
GO

-- 建立房間表
CREATE TABLE Room (
    room_id INT IDENTITY(1,1) PRIMARY KEY,
    room_type INT NOT NULL CHECK (room_type IN (1,2,3,4,6)),
    room_description NVARCHAR(MAX),
    room_image NVARCHAR(255)
);
GO

-- 建立訂房表
CREATE TABLE Booking (
    booking_id INT IDENTITY(1,1) PRIMARY KEY,
    user_id INT NOT NULL,
    room_id INT NOT NULL,
    date_start DATE NOT NULL,
    date_end DATE NOT NULL,
    pay_type NVARCHAR(50),
    FOREIGN KEY (user_id) REFERENCES Users(user_id),
    FOREIGN KEY (room_id) REFERENCES Room(room_id),
    CHECK (date_end >= date_start)
);
GO
