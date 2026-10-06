# 資料庫系統設計期末專案：嘉義大飯店訂房管理系統

## 🔗 相關連結
* **系統 Demo 影片**：[YouTube 介紹影片](https://youtu.be/SJEEjwOD3RQ)

## 🏨 專案簡介
本專案為資料庫系統設計課程之期末專案，開發了一套名為「嘉義大飯店」的訂房管理系統。系統旨在提供使用者一個完整、直覺的線上訂房體驗，並同時為飯店管理者提供後台數據分析、房務管理與營運監控功能。專案深入探討了關聯式資料庫的正規化設計，並實作了高併發下的防超賣機制。

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

## ✨ 系統核心功能與 SQL 進階應用
系統不僅涵蓋基礎的 CRUD，更透過嚴謹的 SQL 語法與資料庫進階特性，確保商業邏輯的正確性與資料的一致性：

### 1. 使用者註冊與權限管理 (RBAC)
* **功能說明**：系統區分「一般房客」與「後台管理員」兩種角色。註冊時系統會驗證帳號是否重複，並對密碼進行雜湊處理。
* **SQL 應用**：利用 `UNIQUE` 限制確保帳號唯一性，並透過角色欄位 (`role_id`) 進行存取控制。

### 2. 空房查詢與防重疊機制
* **功能說明**：使用者可根據「入住日期」、「退房日期」與「房型」進行搜尋。系統能精準過濾掉日期衝突的房間。
* **SQL 應用**：為避免訂房日期重疊，系統使用子查詢與邏輯條件，確保過濾出**在指定期間內未被預訂**的房間。
  ```sql
  SELECT * FROM Room WHERE room_id NOT IN (
      SELECT room_id FROM Booking 
      WHERE NOT (date_end <= @user_check_in_date OR date_start >= @user_check_out_date)
  )
  ```
### 3. 資料庫交易控制 (Transaction) 與防超賣機制
* **功能說明**：當熱門節日有多位使用者同時搶訂同一間房時，系統能有效防止「超賣 (Overbooking)」的嚴重錯誤。
* **SQL 應用**：利用 SQL Server 的 `BEGIN TRAN`、`COMMIT` 與 `ROLLBACK`，確保「檢查空房」與「寫入訂單」這兩個動作具備原子性 (Atomicity)。若中途發生衝突，則自動退回交易。

### 4. 訂單狀態機與觸發器 (Trigger) 應用
* **功能說明**：訂單具備完整的生命週期（如：待付款、已確認、已入住、已退房、已取消）。當房客取消訂單時，系統需自動記錄變更日誌。
* **SQL 應用**：建立 `AFTER UPDATE` 的資料庫觸發器 (Trigger)。當 `Booking` 表內的狀態被更新為「已取消」時，觸發器會自動將該筆紀錄寫入 `Booking_Log` 日誌表中，方便未來稽核。

### 5. 會員中心：訂單管理與附加服務
* **功能說明**：會員可於專屬頁面檢視個人訂單。在入住前，可修改入住日期、變更付款方式，或加購附加服務（如加床、早餐）。
* **SQL 應用**：透過主檔與明細檔 (`Booking` & `Booking_Details`) 的關聯設計，執行靈活的 `UPDATE` 與 `DELETE` 操作，維持資料正規化。

### 6. 後台房型與房態管理 (CRUD)
* **功能說明**：管理員專屬後台可動態新增房型、修改房間設施描述、調整定價，以及標記房間狀態（如：清潔中、維修中）。
* **SQL 應用**：完整的 `INSERT`, `UPDATE`, `DELETE` 實作，並搭配軟刪除 (Soft Delete) 概念（如 `is_active = 0`），確保過往的訂單紀錄不會因為房型被刪除而變成孤兒資料 (Orphan Data)。

### 7. 房客評論與星級評鑑系統
* **功能說明**：退房後的房客可對房間寫下評價與星級，評論將顯示於前台，提供其他旅客參考。
* **SQL 應用**：透過多表 `JOIN` 語法，將 `Reviews`、`Users` 與 `Room` 資料表關聯撈取。
  ```sql
  SELECT 
      u.user_name,
      rm.room_description,
      r.rating,
      r.comment,
      r.review_date
  FROM Reviews r
  JOIN Users u ON r.user_id = u.user_id
  JOIN Room rm ON r.room_id = rm.room_id
  ORDER BY r.review_date DESC;
  ```

### 8. 營收分析與預存程序 (Stored Procedure)
* **功能說明**：管理員可檢視每月營收與各房型入住率報表。為提升查詢效能，將複雜的報表運算邏輯直接交由資料庫引擎處理。
* **SQL 應用**：將報表查詢封裝為 `CREATE PROCEDURE`，結合 `GROUP BY`、`DATEDIFF` 與聚合函數 (`SUM`, `COUNT`)。後端只需呼叫預存程序並傳入年月參數，即可大幅降低網路傳輸與伺服器運算負擔。
  ```sql
  -- 營收計算核心邏輯範例
  SELECT 
      FORMAT(b.date_start, 'yyyy-MM') AS month_str,
      COUNT(b.booking_id) AS total_orders,
      SUM(DATEDIFF(DAY, b.date_start, b.date_end) * r.room_price) AS total_revenue
  FROM Booking b
  JOIN Room r ON b.room_id = r.room_id
  WHERE b.status = 'Completed'
  GROUP BY FORMAT(b.date_start, 'yyyy-MM')
  ORDER BY month_str DESC;
  ```
