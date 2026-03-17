from flask import Flask, render_template, redirect, url_for
from config import Config
from controllers.auth_controller import auth_bp
from controllers.room_controller import room_bp
# from controllers.admin_controller import admin_bp (之後有寫再打開)

app = Flask(__name__)
app.config.from_object(Config)
# 註冊 Blueprints (Prefix 是網址前綴，可加可不加)
app.register_blueprint(auth_bp, url_prefix='/auth') # 網址會變成 /auth/login
app.register_blueprint(room_bp, url_prefix='/rooms') # 網址會變成 /rooms/search

@app.route('/')
def index():
    # 載入外殼，外殼會自己去載入 home (index.html)
    return render_template('app_shell.html') 

# 確保你有這個 route (原本叫 hotel_homePage 或 home)
# 這裡不需要動，保持原樣，只要確認 function name 對應得到
@app.route('/index', methods=['GET', 'POST'])
def home():
    return render_template('index.html')

if __name__ == '__main__':
    app.run(debug=True)