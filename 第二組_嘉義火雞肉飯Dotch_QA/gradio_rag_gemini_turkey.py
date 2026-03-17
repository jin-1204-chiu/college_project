## pip install langchain_google_genai

import os
import sys
import hashlib
import google.generativeai as genai
import gradio as gr
from langchain_google_genai import ChatGoogleGenerativeAI, GoogleGenerativeAIEmbeddings
from langchain_chroma import Chroma
from langchain_community.document_loaders import PyPDFLoader
from langchain.text_splitter import RecursiveCharacterTextSplitter
from langchain.chains import RetrievalQA
#偷gradio_gemini_ncyu.py
from dotenv import load_dotenv
load_dotenv()  # Load environment variables

#新加的，用來網頁搜尋
import requests
from bs4 import BeautifulSoup

#歷史紀錄追蹤
import re

from dotenv import load_dotenv
load_dotenv()  # Load environment variables
# 設定 API 金鑰
GOOGLE_API_KEY = os.getenv("GOOGLE_API_KEY")
genai.configure(api_key=GOOGLE_API_KEY)

# 初始化 Gemini LLM ## gemini-1.5-pro gemini-1.5-flash models/gemini-2.0-flash-001
llm = ChatGoogleGenerativeAI(model="models/gemini-1.5-flash")
embeddings = GoogleGenerativeAIEmbeddings(model="models/embedding-001")

#chat = llm.start_chat(history=[])

# 向量資料庫存儲目錄
persist_directory = "g_gemini_turkey_chroma_vectorstore"
# PDF 文件路徑
pdf_path = "./turkey_rice_QA.pdf"

# **新增常見問題按鈕**
common_questions = [
    "嘉義火雞肉飯的特色是什麼？",
    "離火車站最近的火雞肉飯?",
    "火雞肉飯的歷史由來？",
    "火雞肉飯和雞肉飯有什麼差別？",
    "Google評價最高的火雞肉飯？"
]
# **問題填入函數**
def fill_question(question):
    return question  # 回傳選擇的問題，填入 Textbox

# 網頁搜尋
def web_scrape_search(question):
    # 參考網頁路徑
    turkey_search_urls = ["https://travel.chiayi.gov.tw/StaticPage/TurkeyRiceOrigin", "https://travel.chiayi.gov.tw/StaticPage/TurkeyRiceFeature"]
    #設定 HTTP 請求的標頭，可以讓爬蟲偽裝成瀏覽器，避免被網站的反爬蟲機制封鎖
    headers = {
        "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/121.0.0.0 Safari/537.36"
    }
    
    combined_content = ""

    for url in turkey_search_urls:
        try:
            response = requests.get(url, headers=headers)
            soup = BeautifulSoup(response.text, "html.parser")
            
            # 擷取網頁主要內容
            page_text = soup.get_text(separator="\n", strip=True)
            combined_content += f"\n--- 來自: {url} ---\n{page_text}\n"
        
        except requests.exceptions.RequestException as e:
            print(f"無法擷取 {url} 的內容。錯誤: {e}")
    
    # 返回前 2000 個字元以避免過長
    return combined_content[:2000]



# 計算 PDF 文件 HASH
def calculate_file_hash(file_path):
    hasher = hashlib.md5()
    with open(file_path, "rb") as f:
        buf = f.read()
        hasher.update(buf)
    return hasher.hexdigest()

# HASH文件存儲路徑
hash_file_path = os.path.join(persist_directory, "pdf_hash.txt")

# 檢查是否需要更新資料庫
def needs_update(pdf_path, hash_file_path):
    new_hash = calculate_file_hash(pdf_path)
    if not os.path.exists(hash_file_path):
        return True, new_hash
    with open(hash_file_path, "r") as f:
        existing_hash = f.read().strip()
    return new_hash != existing_hash, new_hash

update_required, new_hash = needs_update(pdf_path, hash_file_path)

if os.path.exists(persist_directory) and not update_required:
    print("---正在加載現有的向量資料庫...")
    vectorstore = Chroma(persist_directory=persist_directory, embedding_function=embeddings)
else:
    print("---PDF 文件已更新，正在重新生成資料庫...")
    if os.path.exists(persist_directory):
        print("---正在刪除舊的向量資料庫...")
        import shutil
        shutil.rmtree(persist_directory)
    
    print("---正在載入文件，請稍候...")
    loader = PyPDFLoader(pdf_path)
    documents = loader.load()
    for i, doc in enumerate(documents):  # 顯示前3頁 [:3]
        print(f"第 {i+1} 頁內容（部分）:\n{doc.page_content}") # [:50]
    
    print(f"---PDF 加載完成，共 {len(documents)} 頁")
    
    print("---正在分割文本...")
    text_splitter = RecursiveCharacterTextSplitter(chunk_size=800, chunk_overlap=150)
    docs = text_splitter.split_documents(documents)
    
    #print("---正在生成向量資料庫...")
    vectorstore = Chroma.from_documents(docs, embeddings, persist_directory=persist_directory)
    
    with open(hash_file_path, "w") as f:
        f.write(new_hash)
    #print("---向量資料庫已保存！")

# 建立檢索問答鏈
retriever = vectorstore.as_retriever()
qa_chain = RetrievalQA.from_chain_type(llm=llm, chain_type="stuff", retriever=retriever)

# 初始化對話歷史
conversation_history = []

# 將問題與回答存入歷史，並且強調「主要對象」
last_subject = None  # 追蹤最新的主題


def detect_main_subject(question, pdf_answer, web_content):
    """ 從問題與資料中找出主要對象 """
    subjects = ["火雞肉飯", "雞肉飯", "嘉義", "店家名稱"]
    
    for sub in subjects:
        if re.search(sub, question) or re.search(sub, pdf_answer) or re.search(sub, web_content):
            return sub  # 返回找到的主題
    return None  # 如果未找到


def rag_chain_with_scraped_web(question):
    global conversation_history, last_subject  # 使用全域對話歷史
    
    # 檢查輸入是否為空
    if not question.strip():
        return chat_history

    # PDF 檢索
    pdf_response = qa_chain.invoke(question)
    pdf_answer = pdf_response["result"]

    # 網頁爬蟲結果
    web_content = web_scrape_search(question)
    
    # 偵測新主題
    new_subject = detect_main_subject(question, pdf_answer, web_content)
    if new_subject and new_subject != last_subject:
        last_subject = new_subject

    # 如果主要對象變更，則更新
    if new_subject and new_subject != last_subject:
        last_subject = new_subject
        conversation_history.append((f"【新的主要對象】：{new_subject}", ""))

    
    # 預設頭像
    user_avatar = "user_image.jpg"  # 使用者頭像
    bot_avatar = "ai_image.jpg"  # 機器人頭像
    
    # 結合內容
    combined_context = (
        f"以下是從 PDF 與網頁檢索到的資料，請基於這些內容回答問題。"
        f"若資料中有相關資訊，請詳細回答。若資料中沒有相關內容，請明確說明無法找到資料。\n\n"
        f"PDF 資料:\n{pdf_answer}\n\n"
        f"網頁搜尋結果:\n{web_content}\n\n"
        f"請根據以上資訊再回答問題：{question}，專業的回答，不需要提到根據提供的資料"
    )
    
    # **建立 prompt，包含最近 10 輪對話歷史**
    prompt = (
    "你是專門回答嘉義火雞肉飯問題的助手，請基於以下資訊與對話歷史回答問題。\n"
    "如果問題中使用了代詞（如「他」或「它」或「她」）或者沒有主要的店名，請根據最近的對話上下文推斷出對應的對象，並基於提供的資料回答，如果有答案就直接顯示，不用再顯其他東西。\n\n"
)
    for q, a in conversation_history[-10:]:  # 取最近 10 輪對話
        prompt += f"使用者: {q}\n"
        prompt += f"助手: {a}\n"
    prompt += f"使用者: {question}\n"
    prompt += f"{combined_context}\n"
    prompt += "助手:"

    # 交由 LLM 處理
    final_response = llm.invoke(prompt)
    
    # 提取回應內容
    response_text = final_response.content if hasattr(final_response, 'content') else str(final_response)

    # 將回答加入 conversation_history 並且在開頭加上名稱
    conversation_history.append((f"小吃貨:{question}",f"雞肉飯小精靈: {response_text}"))
        
    # Return the updated conversation_history
    return conversation_history
    



with gr.Blocks(
    theme=gr.themes.Origin(
        secondary_hue="rose",
        neutral_hue="red",
        text_size=gr.themes.Size(
            lg="22px", md="20px", sm="18px",
            xl="28px", xs="15px", xxl="32px", xxs="14px"
        ),
        spacing_size="lg",
        radius_size="sm",
    )
) as demo:
    
    conversation_state = gr.State([])  # 儲存對話紀錄的 State
    
    gr.Markdown("<center><h1>【 嘉義火雞肉飯Dotch Q&A 🍚】</h1></center>")
    gr.Markdown("<center>根據提供的嘉義火雞肉飯Dotch_QA文件(turkey_rice_QA.pdf)以及嘉義市政府網站回答問題</center>")
    
    # 移除 equal_height 參數，讓左側區域保持原本高度
    with gr.Row():
        with gr.Column():
            # 問題輸入框，保持原本的行數（高度）
            question = gr.Textbox(
                lines=2,
                label="question",
                placeholder="請輸入你的問題"
            )

            # 在同一行中，先 Clear 再 Submit（Clear 在左，Submit 在右，並設定主色）
            with gr.Row():
                clear_btn = gr.Button("Clear")
                submit_btn = gr.Button("Submit", variant="primary")
            
            # 範例：點擊會把文字帶入 question
            gr.Examples(
                examples=[[q] for q in common_questions],
                inputs=question,
                label="Examples"
            )
        
        with gr.Column():
            # 聊天紀錄，調整高度保持目前輸出長度
            chatbot = gr.Chatbot(
                label="聊天紀錄",
                avatar_images=("user_image.jpg", "ai_image.jpg"),
                height=550,  # 你滿意的 output 高度
            )
    
    # 點擊 Submit：呼叫 rag_chain_with_scraped_web
    submit_btn.click(
        fn=rag_chain_with_scraped_web,
        inputs=question,
        outputs=chatbot
    )

    # 點擊 Clear：清空對話
    clear_btn.click(
        fn=lambda: [],
        inputs=None,
        outputs=chatbot,
        queue=False
    )

demo.launch(share=True)
