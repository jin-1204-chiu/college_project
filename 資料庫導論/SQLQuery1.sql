-- 確定沒有使用其它的資料庫
USE master
GO
IF NOT EXISTS (
   SELECT name
   FROM sys.databases
   WHERE name = N'TutorialDB'
)

-- 沒有TutorialDB資料庫，所以建一個
CREATE DATABASE [TutorialDB]
GO
