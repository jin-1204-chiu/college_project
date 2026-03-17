嘉義火雞肉飯Dotch_QA
簡介：
我們的 AI 問答系統專門回答有關嘉義火雞肉飯的問題，根據內部的 PDF 內容以及外部網頁資訊進行回應，並能根據歷史紀錄持續對話。在 Gradio 介面中，提供與嘉義火雞肉飯相關的問題按鈕，使用者可直接點擊提問，快速獲得更多雞肉飯的資訊。介面經過美化，包括按鈕顏色調整、標題、頭像等設計，提升整體使用體驗。

整體的檢索內容檔案：
pdf：turkey_rice_QA.pdf
word：turkey_rice_QA.word

程式功能說明：
網頁搜尋：
使用 BeautifulSoup 和 requests 套件讀取兩個外部網頁的內容。讀取時以換行符號分隔各區塊，並移除首尾的空白字元。同時限制回傳字數，以避免內容過長。

歷史紀錄：
可追蹤使用者先前詢問的店家，方便延續問題討論。使用 conversation_history 陣列儲存每一輪問答，當紀錄達到 10 輪時，會自動刪除最舊的問答。

內容整合與生成：
使用 gemini-1.5-flash 模型，透過 rag_chain_with_scraped_web 將 PDF 和網頁內容整合。設計的 prompt 可讓回答更貼近預期需求，並於回應中標示使用者和 AI 的名稱。

Gradio介面：
介面經過美化，調整色調、字體大小、邊框等元素。同時提供與雞肉飯相關的範例問題按鈕，方便快速提問。問答框中會顯示使用者與 AI 的名稱和頭像，也可以看到之前詢問的問題及回應，這個部分結合了ollama+gradio影片的gradio介面，提升閱讀的便利性和直觀性。問答框可以

使用說明：
1. 下載python，選最新版本即可。
2. 前往 Google AI Studio 建立 API 金鑰，並將金鑰放入 .env 檔案。
2. 建立資料夾，放置 Python 檔案、.env 檔案及 PDF 檔案。
3. 開啟終端機並建立虛擬環境。
4. 下載 requirements.txt 並安裝所需套件。
5. 在虛擬環境中執行python檔。
6. 點擊終端機中顯示的網址，即可進入 AI 問答介面。

影片連結：
https://youtu.be/oRWUBXk3HCY?feature=shared
