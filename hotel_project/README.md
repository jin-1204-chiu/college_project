# 資料庫系統設計期末專案：嘉義大飯店訂房管理系統

## 🔗 相關連結
* **系統 Demo 影片**：[YouTube 介紹影片](https://youtu.be/SJEEjwOD3RQ)

## 🏨 專案簡介
本專案為資料庫系統設計課程之期末專案，開發了一套名為「嘉義大飯店」的訂房管理系統。系統旨在提供使用者一個完整、直覺的線上訂房體驗，並同時為飯店管理者提供後台數據分析與房務管理功能。

## 🏗️ 系統架構設計 (MVC 架構)
本系統採用 **MVC (Model-View-Controller)** 軟體設計模式進行開發，確保程式碼的結構清晰且易於維護：

* **View (視圖層 - 前端介面)**
  * 使用 **Jinja2** 模板引擎負責 HTML 結構的動態渲染。
  * 導入 **Bootstrap 5** 框架負責 UI 樣式、排版與響應式設計。
* **Controller (控制層 - 路由與邏輯)**
  * 使用 **Flask** 輕量級網頁框架進行路由設定 (`Blueprints`) 與後端商業邏輯控制。
  * 搭配 **jQuery** 處理前端的 AJAX 請求與事件。
* **Model (模型層 - 資料庫互動)**
  * 使用 **SQL Server** 作為關聯式資料庫。
  * 透過 **PyODBC** 套件執行 SQL 指令並管理資料庫連線。

## ✨ 系統核心功能與 SQL 應用
系統實作了以下主要功能，並透過嚴謹的 SQL 語法確保資料正確性：

### 1. 註冊 / 登入 / 登出功能
* **功能說明**：使用者必須填寫使用者名稱、帳號及密碼（不得為空）才能完成註冊。系統會驗證帳號是否重複，並提供 AJAX 非同步登入功能。
* **SQL 應用**：利用 `UNIQUE` 限制確保帳號唯一性，並透過 `INSERT INTO` 與 `SELECT` 語法處理註冊與登入驗證。

### 2. 空房查詢 (防重疊機制)
* **功能說明**：使用者可根據「入住日期」、「退房日期」與「房型」進行搜尋。
* **SQL 應用**：為避免訂房日期重疊，系統使用子查詢與邏輯條件，確保過濾出**在指定期間內未被預訂**的房間。
  ```sql
  SELECT * FROM Room WHERE room_id NOT IN (
      SELECT room_id FROM Booking
      WHERE NOT (date_end < @user_check_in_date OR date_start > @user_check_out_date)
  )

  ### 3. 會員中心：編輯與取消訂單
* **功能說明**：會員可於專屬頁面 (`/auth/profile`) 檢視個人訂單。在入住前，會員有權利取消訂單或修改入住日期及付款方式。
* **SQL 應用**：
  * 系統執行 `INSERT` 新增訂單、`UPDATE` 修改訂單，或 `DELETE` 移除訂單資料。
  ```sql
  -- 新增訂單
  INSERT INTO Booking (user_id, room_id, date_start, date_end, pay_type)
  VALUES (@user_id, @room_id, @date_start, @date_end, @pay_type);

  -- 刪除訂單
  DELETE FROM Booking
  WHERE booking_id = @booking_id;

  -- 更新訂單資訊
  UPDATE Booking
  SET date_start = @date_start,
      date_end = @date_end,
      pay_type = @pay_type
  WHERE booking_id = @booking_id;

  ### 4. 房客評論系統
* **功能說明**：房客可對入住過的房間寫下評價與星級，評論將顯示於前台，並供後台管理員讀取。
* **SQL 應用**：透過 `JOIN` 語法，將 `Reviews` 資料表與 `Users`、`Room` 資料表關聯，以便完整呈現。
  ```sql
  -- 寫入新評論 (Insert Review)
  INSERT INTO Reviews (user_id, room_id, rating, comment, review_date)
  VALUES (@user_id, @room_id, @rating, @comment, @review_date);

  -- 後台撈取評論 (包含使用者名稱與房型)
  SELECT 
      u.user_name,
      rm.room_description,
      r.rating,
      r.comment
  FROM Reviews r
  JOIN Users u ON r.user_id = u.user_id
  JOIN Room rm ON r.room_id = rm.room_id
  ORDER BY r.review_date DESC;

  ### 5. 管理員後台與營收分析
* **功能說明**：管理員可進入後台檢視房間狀態與統計數據，系統能自動計算並統整每月的總訂單數與營收。
* **SQL 應用**：利用 `GROUP BY` 與日期格式化，結合 `DATEDIFF` 計算入住天數並乘以房價，彙總當月總營收。
  ```sql
  SELECT
      FORMAT(b.date_start, 'yyyy-MM') AS month_str,
      COUNT(b.booking_id) AS total_orders,
      -- 計算(退房-入住天數) * 房價 = 該單總金額
      SUM(DATEDIFF(DAY, b.date_start, b.date_end) * r.room_price) AS total_revenue
  FROM Booking b
  JOIN Room r ON b.room_id = r.room_id
  GROUP BY FORMAT(b.date_start, 'yyyy-MM')
  ORDER BY month_str DESC;
