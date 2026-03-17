DELETE FROM Booking;
DELETE FROM Room;
DBCC CHECKIDENT ('Room', RESEED, 0);  -- 讓下次插入從 1 開始

INSERT INTO Room (room_type, room_description, room_image)
VALUES 
(1, '一人房，附早餐，乾淨明亮', 'room1_1.jpg'),
(1, '一人房，有陽台，乾淨明亮', 'room1_2.jpg'),
(1, '一人房，乾淨明亮', 'room1_3.jpg'),
(2, '二人房，附早餐，乾淨明亮', 'room2_1.jpg'),
(2, '二人房，有陽台，乾淨明亮', 'room2_2.jpg'),
(2, '二人房，乾淨明亮', 'room2_3.jpg'),
(3, '三人房，附早餐，乾淨明亮', 'room3_1.jpg'),
(3, '三人房，有陽台，乾淨明亮', 'room3_2.jpg'),
(3, '三人房，乾淨明亮', 'room3_3.jpg'),
(4, '四人房，附早餐，乾淨明亮', 'room4_1.jpg'),
(4, '四人房，有陽台，乾淨明亮', 'room4_2.jpg'),
(4, '四人房，乾淨明亮', 'room4_3.jpg'),
(6, '六人房，附早餐，乾淨明亮', 'room6_1.jpg'),
(6, '六人房，有陽台，乾淨明亮', 'room6_2.jpg'),
(6, '六人房，乾淨明亮', 'room6_3.jpg');


-- 插入訂房資料
INSERT INTO Booking (user_id, room_id, date_start, date_end, pay_type)
VALUES 
  (1, 1, '2025-07-21', '2025-08-02', '現金'),
  (2, 4, '2025-06-21', '2025-06-23', '信用卡'), --二人房
  (3, 8, '2025-06-21', '2025-06-23', '現金'); --三人房