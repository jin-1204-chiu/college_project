from flask import Blueprint, render_template, request, session, jsonify, redirect, url_for
from models.room_model import RoomModel
from models.booking_model import BookingModel
from models.user_model import UserModel
from datetime import date, datetime

room_bp = Blueprint('room', __name__)

@room_bp.route('/search', methods=['GET', 'POST'])
def search_rooms():
    # 結合原本的 GET 與 POST 邏輯
    rooms = []
    today = date.today().strftime('%Y-%m-%d')
    date_start = request.form.get('date_start') or request.args.get('date_start', '')
    date_end = request.form.get('date_end') or request.args.get('date_end', '')
    room_type = request.form.get('room_type') or request.args.get('room_type', '')

    if request.method == 'POST' or (date_start and date_end):
        rooms = RoomModel.search_available(date_start, date_end, room_type)
        
    return render_template('rooms/list.html', 
                         rooms=rooms, 
                         date_start=date_start, 
                         date_end=date_end, 
                         room_type=room_type, 
                         current_date=today)

@room_bp.route('/booking_confirm', methods=['POST'])
def booking_confirm():
    if 'account' not in session:
        return 'unauthorized'

    room_id = request.form['room_id']
    date_start = request.form['date_start']
    date_end = request.form['date_end']
    pay_type = request.form['payment']
    
    user = UserModel.get_user(session['account'])
    
    if user and BookingModel.create(user.user_id, room_id, date_start, date_end, pay_type):
        return 'success'
    return 'fail'

# 訂單操作 (取消、編輯)
@room_bp.route('/cancel', methods=['POST'])
def cancel_booking():
    data = request.get_json()
    if BookingModel.cancel(data.get('booking_id')):
        return jsonify({'status': 'success'})
    return jsonify({'status': 'fail', 'message': '取消失敗'})

@room_bp.route('/edit', methods=['POST'])
def edit_booking():
    booking_id = request.form['booking_id']
    room_id = request.form['room_id']
    date_start = request.form['date_start']
    date_end = request.form['date_end']
    pay_type = request.form['pay_type']
    
    # 日期驗證
    if datetime.strptime(date_end, '%Y-%m-%d') <= datetime.strptime(date_start, '%Y-%m-%d'):
         return "<script>alert('退房日錯誤'); window.history.back();</script>"

    if BookingModel.update(booking_id, room_id, date_start, date_end, pay_type):
        return redirect(url_for('auth.profile'))
    else:
        return "<script>alert('日期衝突'); window.history.back();</script>"