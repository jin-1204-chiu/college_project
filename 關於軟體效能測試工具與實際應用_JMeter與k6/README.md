# 軟體工程實務期末報告：那一年，我們一起搞垮的SERVER
**關於軟體效能測試工具與實際應用**

---

## 我的貢獻
* **ParaBank JMeter 情境展示**：負責執行並展示使用 JMeter 對 ParaBank 進行壓力測試。
* **(Bonus) 增加 ParaBank 功能**：擴充 ParaBank 的應用功能。
* **(Bonus) 針對 ParaBank 新增功能進行 JMeter 測試**：對額外擴充的功能進行效能測試與驗證。

---

## 📌 專案前言與背景
在軟體系統上線前，為了避免因高流量導致伺服器崩潰（如常見的 `500 Internal Server Error` 或 `502 Bad Gateway`），開發團隊必須進行壓力測試（Stress Testing）與負載測試（Load Testing）。本專案旨在探討並實作兩大主流測試工具：**Apache JMeter** 與 **Grafana k6**，評估系統在不同使用人數與流量條件下的效能表現。

## 🛠️ 測試工具介紹與比較
本報告深入探討了兩款工具的特性與併發架構模型：

### Apache JMeter
* **特性**：由 Apache 開發的開源工具，純 Java 撰寫，歷史悠久且提供功能齊全的圖形化介面 (GUI)。
* **架構**：採用傳統的 Thread-based 模型。每個虛擬用戶對應一條 JVM Thread（與 OS Thread 1:1 綁定）。單個執行緒約佔 1-8MB 記憶體，建立大量併發時容易吃滿系統資源且 Context Switch 成本較高。

### Grafana k6
* **特性**：以 Go 語言開發，使用 JavaScript 撰寫測試腳本，為現代化、開發者體驗佳的輕量級 CLI 測試工具。
* **架構**：採用 Goroutine-based 的極度輕量模型（GMP 模型）。每個虛擬用戶對應一個 Goroutine，初始記憶體僅需約 2KB，透過 Go Runtime 排程於少量的 OS Thread 上，Context Switch 在 User Space 完成，能輕鬆達到數萬併發量且資源佔用極小。

## 🏦 ParaBank 壓力測試情境設計
我們以開源的虛擬銀行系統 **ParaBank** 作為測試目標，並模擬了真實使用者的操作行為（如：查看餘額、存款、提款、轉帳、登出），針對不同情境進行測試。

**測試情境定義**：
* **正常狀況**：100 個虛擬用戶 (Threads)。
* **高併發 (High Concurrency)**：瞬間大量請求，設定為 300 至 2000 個虛擬用戶，無緩衝時間 (Ramp-up = 0)，考驗系統的連線池配置。
* **高負載 (High Load)**：持續密集的請求，設定為 300 至 2000 個虛擬用戶，帶有緩衝時間，考驗系統吞吐極限與資源回收。

### Nginx 伺服器配置 (代理設定)
在測試架構中，我們使用 Nginx 作為反向代理，將流量導向後端的 Tomcat (Port 8080)。相關配置如下：
```nginx
events { worker_connections 1024; }

http {
    upstream backendbank {
        server host.docker.internal:8080; 
    }

    server {
        listen 80;

        location / {
            proxy_pass http://backendbank;
            proxy_read_timeout 3s;
            proxy_connect_timeout 3s;
        }
    }
}
```

*在此設定中，我們刻意將 `proxy_read_timeout` 與 `proxy_connect_timeout` 設為 3 秒，當系統壓力過載處理不及時，就會觸發 504 Gateway Timeout 錯誤。*

## 🚨 錯誤分析與解決方案
在極限壓力測試下，系統出現了以下錯誤：
* **504 Gateway Timeout**：因請求過多，Tomcat 無法在 Nginx 設定的 3 秒內給予回應。
* **資料庫錯亂**：在轉帳併發測試時，若無完善的同步機制 (Synchronized)，會出現扣款金額異常（如帳戶金額出現負數）的 Race Condition 問題。

**解決方法 (防禦性編程)**：
我們在後端 Java 程式碼中實作了執行緒安全的同步機制與防禦檢查：
1. 使用 `synchronized` 關鍵字鎖定 `withdraw` 與 `transfer` 方法。
2. 加入餘額判斷：`if (account.getBalance().compareTo(amount) < 0)`，若餘額不足則直接拋出例外中斷交易，避免產生負數與資料不一致。

## 📝 總結
效能測試是系統防禦的第一線，能提早發掘系統瓶頸。在工具選擇上，JMeter 適合傳統測試與習慣 GUI 操作的團隊；而 k6 則憑藉其極度輕量的 Goroutine 架構與純程式碼腳本，能更輕易地模擬出高併發環境，且完美契合現代 CI/CD 的開發流程。
