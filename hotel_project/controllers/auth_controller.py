from flask import Blueprint, request, session, jsonify, render_template, redirect, url_for
from models.user_model import UserModel
from models.booking_model import BookingModel

# 定義 Blueprint
auth_bp = Blueprint('auth', __name__)

# --- 登入 (只處理 POST, 給 AJAX 用) ---
@auth_bp.route('/login', methods=['POST'])
def login():
    account = request.form.get('account')
    password = request.form.get('password')
    
    user = UserModel.get_user(account, password)

    if user:
        session['user_name'] = user.user_name
        session['account'] = user.account
        return jsonify({'status': 'success'})
    else:
        return jsonify({'status': 'fail'})

# --- 註冊 (只處理 POST, 給 AJAX 用) ---
@auth_bp.route('/register', methods=['POST'])
def register():
    user_name = request.form['user_name']
    account = request.form['account']
    password = request.form['password'] 
    
    if UserModel.create_user(user_name, account, password):
        return "success"
    return "fail"

# --- 登出 ---
@auth_bp.route('/logout')
def logout():
    session.clear()
    return redirect(url_for('home'))

# --- 【補回這裡】會員中心 / 訂單清單 (原本的 shopping_list) ---
@auth_bp.route('/profile')
def profile():
    # 如果沒登入，導回首頁
    if 'account' not in session:
        return redirect(url_for('home'))

    user = UserModel.get_user(session['account'])
    bookings = []
    
    if user:
        bookings = BookingModel.get_by_user(user.user_id)

    # 記得確認你有把原本的 shopping_list.html 改名搬到 templates/auth/profile.html
    return render_template('auth/profile.html', bookings=bookings, user=session.get('user_name'))