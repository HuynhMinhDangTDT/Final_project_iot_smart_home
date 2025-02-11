from django.shortcuts import render , redirect, get_object_or_404
from .models import information
from django.contrib import messages
# from django.contrib.auth import authenticate,login,logout
from django.contrib.auth.models import User
from django.contrib.auth.decorators import login_required
from django.http import JsonResponse
from django.http import HttpResponse, HttpResponseRedirect
from django.template import loader
from django.urls import reverse
import pyrebase
from django.contrib.auth.models import User
from rest_framework import authentication
from rest_framework import exceptions
import firebase_admin as admin
import firebase_admin.auth as auth
from django.contrib import auth

# from smarthome_app.middleware import FirebaseAuthMiddleware
# from server_smart_skylight import  server
# from real_time_fire_detection import fire_detection

import firebase_admin
from firebase_admin import credentials, db

config = {
    'apiKey': "AIzaSyCSTpQEqVcR-lW5DsgE5_Kqn7MrV6OZSx8",
    'authDomain': "iot-smart-home-a7c95.firebaseapp.com",
    'databaseURL': "https://iot-smart-home-a7c95-default-rtdb.asia-southeast1.firebasedatabase.app",
    'projectId': "iot-smart-home-a7c95",
    'storageBucket': "iot-smart-home-a7c95.appspot.com",
    'messagingSenderId': "327764472304",
    'appId': "1:327764472304:web:74cc73510c2b07e3686d35",
    'measurementId': "G-QSPT43DZY1"
}


cred = credentials.Certificate("static/iot-smart-home-a7c95-firebase-adminsdk-3ky17-9bdd30ab30.json")
firebase_admin.initialize_app(cred, {
    'databaseURL': "https://iot-smart-home-a7c95-default-rtdb.asia-southeast1.firebasedatabase.app/"
})
firebase = pyrebase.initialize_app(config)
authe = firebase.auth()
DataBase = firebase.database()

def LoginPage(request):
    # if request.method == 'POST':
    #     # username=request.POST.get('username')
    #     email = request.POST.get('email')
    #     password = request.POST.get('password')
    #     try:
    #         # if there is no error then signin the user with given email and password
    #         user = authe.sign_in_with_email_and_password(email, password)
    #         # print(user['idToken'])
    #         id_token = user['idToken']
    #         # print(id_token)
    #         # session_id = user['idToken']
    #         # request.session['uid'] = str(session_id)
    #         idtoken = request.session['uid']
    #         a = authe.get_account_info(idtoken)
    #         # print(a)
    #         if id_token != None:
    #             FirebaseAuthMiddleware.process_request(self, auth_header=id_token)
    #         # print("id" +" " + ' : ' + str(idtoken))
    #         # a = authe.get_account_info(idtoken)
    #         # print(a)
    #         # print(request.session['uid'])
            
    #         # return HttpResponseRedirect(reverse('dashboard'))
    #         return redirect(f'/dashboard/?token={id_token}')
    #     except:
    #         messages.error(request, ('userID or Password is incorrect!!!!'))
    #         return render(request, "login.html")
    #     # user=authenticate(request,username=username,password=pass1)
    #     # if user is not None:
    #     #     login(request,user)
    #     #     # return redirect(request.GET.get('next'))
    #     #     return HttpResponseRedirect(reverse('dashboard'))
    #     # else:
    #     #     messages.success(request, ('userID or Password is incorrect!!!!'))
    #     #     return redirect('login')
    #     print(user['idToken'])
    #     session_id = user['idToken']
    #     request.session['uid'] = str(session_id)

    #     idtoken = request.session['uid']
    #     print("id" +" " + ' : ' + str(idtoken))
    #     a = authe.get_account_info(idtoken)
    #     print(a)
    #     a = a['Users']
    #     # a = a[0]
    #     # a = a['localId']
    # else:
    #     return render(request, 'login.html')
    if request.method == 'POST':
        # response = HttpResponse("Cookies Cleared")
        # response.delete_cookie('sessionid')  # Delete specific cookie
        email = request.POST.get('email')
        password = request.POST.get('password')
        
        try:
            # Attempt to sign in with Firebase
            user = authe.sign_in_with_email_and_password(email, password)
            id_token = user['idToken']
            
            # Store the id_token in session
            request.session['uid'] = id_token
            
            # Redirect to the dashboard or any page after successful login
            response = redirect('/dashboard/')
            # response.set_cookie('idToken', id_token)
            response.set_cookie('idToken', id_token, httponly=True, max_age = 3600)
            return response
            # return redirect(f'/dashboard/?token={id_token}')
        
        except:
            messages.error(request, "Email or password is incorrect!")
            return render(request, "login.html")
    else:
        return render(request, "login.html")


def LogoutPage(request):
    # logout(request)
    # return redirect('login')
    # try:
    #     del request.session['uid']
    # except:
    #     pass
    auth.logout(request)
    # Clear the session
    request.session.flush()  # Clear all session data
    response = redirect('/login/')
    response.delete_cookie('idToken')  # Delete the token cookie
    # Redirect to login page
    # return redirect('login')
    return response
    # return render(request, "Login.html")

# def signUp(request):
#     return render(request, "Registration.html")

# def postsignUp(request):
#      email = request.POST.get('email')
#      passs = request.POST.get('pass')
#      name = request.POST.get('name')
#      try:
#         # creating a user with the given email and password
#         user=authe.create_user_with_email_and_password(email,passs)
#         uid = user['localId']
#         idtoken = request.session['uid']
#         print(uid)
#      except:
#         return render(request, "Registration.html")
#      return render(request,"Login.html")

# @login_required(login_url='login')
# @login_required

def dashboard(request):
    # Check if the user is logged in
    id_token = request.session.get('uid')
    
    if not id_token:
        return redirect('login')  # If no token, redirect to login
    
    try:
        user_info = authe.get_account_info(id_token)
        email_verified = user_info['users'][0]['emailVerified']
        print(email_verified)
        email = user_info['users'][0]['email']
        print(email)
        if not email_verified:
            messages.error(request, "Please verify your email before logging in.")
            return redirect('login')
    except Exception as e:
        messages.error(request, f"Error retrieving account info: {e}")
        return redirect('login')

    data = DataBase.child("Users").get()
    # print(data)
    # print(user_info)
    
    users_ref = db.reference("Users")
    users_data = users_ref.get()
    # print(users_data)
    # Tìm user dựa trên email đăng nhập
    humidity = 0
    temperature = 0
    Total_Energy_Use = 0
    toggle_state0 = False
    toggle_state1 = False
    toggle_state2 = False
    toggle_state3 = False
    toggle_state4 = False
    toggle_state5 = False
    
    toggle_status0 = "false"
    toggle_status1 = "false"
    toggle_status2 = "false"
    toggle_status3 = "false"
    toggle_status4 = "false"
    toggle_status5 = "false"
    
    name_device0 = ""
    name_device1 = ""
    name_device2 = ""
    name_device3 = ""
    name_device4 = ""
    name_device5 = ""
    
    if request.method == "POST":
        # Cập nhật trạng thái toggle
        toggle_state0 = request.POST.get("toggle0")
        toggle_state1 = request.POST.get("toggle1")
        toggle_state2 = request.POST.get("toggle2")
        toggle_state3 = request.POST.get("toggle3")
        toggle_state4 = request.POST.get("toggle4")
        toggle_state5 = request.POST.get("toggle5")
        
        if users_data:
            for user_id, user_info in users_data.items():
                if user_info.get("email") == email:
                    print(f"User ID: {user_id}")
                    print(f"Name: {user_info.get('name')}")
                    print(f"Phone: {user_info.get('phone')}")
                    # Truy xuất thông tin DHT11
                    dht11_data = user_info.get("DHT11", {})
                    if dht11_data:
                        humidity = dht11_data.get('doam', 'N/A')
                        temperature = dht11_data.get('nhietdo', 'N/A')
                        print(f"DHT11 - Do am: {humidity}")
                        print(f"DHT11 - Nhiet do: {temperature}")

                    # Truy xuất thông tin công suất tiêu thụ
                    power_consumption = user_info.get("capacity", {})
                    if power_consumption:
                        Total_Energy_Use = power_consumption.get('CongSuatTieuThu', 'N/A')
                        print(f"Công suất tiêu thụ: {Total_Energy_Use}")

                    # Truy xuất thông tin thiết bị
                    devices = user_info.get("device", [])
                    for device in devices:
                        print(f"Device ID: {device.get('id')}, Name: {device.get('name')}, Status: {device.get('status')}")
                    
                    try:
                        name_device0 = devices[0]['name']
                    except:
                        name_device0 = "Chua duoc them vao"
                    try:
                        name_device1 = devices[1]['name']
                    except:
                        name_device1 = "Chua duoc them vao"
                    try:
                        name_device2 = devices[2]['name']
                    except:
                        name_device2 = "Chua duoc them vao"
                    try:
                        name_device3 = devices[3]['name']
                    except:
                        name_device3 = "Chua duoc them vao"
                    try:
                        name_device4 = devices[4]['name']
                    except:
                        name_device4 = "Chua duoc them vao"
                    try:
                        name_device5 = devices[5]['name']
                    except:
                        name_device5 = "Chua duoc them vao"
                    
                    print(name_device0)
                    print(name_device1)
                    print(name_device2)
                    print(name_device3)
                    print(name_device4)
                    print(name_device5)
                    
                    if name_device0 != "Chua duoc them vao":
                        if toggle_state0 is None:
                            toggle_status0 = "false"
                            device_ref = users_ref.child(user_id).child("device").child("0")
                            device_ref.update({
                                "status": toggle_status0
                            })
                            # user_ref = DataBase.child("Users").child(user.key())
                            # user_ref.child("device").child("0").update({"status": toggle_status0})
                        else:
                            toggle_status0 = toggle_state0
                            device_ref = users_ref.child(user_id).child("device").child("0")
                            device_ref.update({
                                "status": toggle_status0
                            })
                            # user_ref = DataBase.child("Users").child(user.key())
                            # user_ref.child("device").child("0").update({"status": toggle_status0})
                    if name_device1 != "Chua duoc them vao":
                        if toggle_state1 is None:
                            toggle_status1 = "false"
                            device_ref = users_ref.child(user_id).child("device").child("1")
                            device_ref.update({
                                "status": toggle_status1
                            })
                            # user_ref = DataBase.child("Users").child(user.key())
                            # user_ref.child("device").child("1").update({"status": toggle_status1})
                        else:
                            toggle_status1 = toggle_state1
                            device_ref = users_ref.child(user_id).child("device").child("1")
                            device_ref.update({
                                "status": toggle_status1
                            })
                            # user_ref = DataBase.child("Users").child(user.key())
                            # user_ref.child("device").child("1").update({"status": toggle_status1})
                    if name_device2 != "Chua duoc them vao":
                        if toggle_state2 is None:
                            toggle_status2 = "false"
                            device_ref = users_ref.child(user_id).child("device").child("2")
                            device_ref.update({
                                "status": toggle_status2
                            })
                            # user_ref = DataBase.child("Users").child(user.key())
                            # user_ref.child("device").child("2").update({"status": toggle_status2})
                        else:
                            toggle_status2 = toggle_state2
                            device_ref = users_ref.child(user_id).child("device").child("2")
                            device_ref.update({
                                "status": toggle_status2
                            })
                            # user_ref = DataBase.child("Users").child(user.key())
                            # user_ref.child("device").child("2").update({"status": toggle_status2})
                    if name_device3 != "Chua duoc them vao":
                        if toggle_state3 is None:
                            toggle_status3 = "false"
                            device_ref = users_ref.child(user_id).child("device").child("3")
                            device_ref.update({
                                "status": toggle_status3
                            })
                            # user_ref = DataBase.child("Users").child(user.key())
                            # user_ref.child("device").child("3").update({"status": toggle_status3})
                        else:
                            toggle_status3 = toggle_state3
                            device_ref = users_ref.child(user_id).child("device").child("3")
                            device_ref.update({
                                "status": toggle_status3
                            })
                            # user_ref = DataBase.child("Users").child(user.key())
                            # user_ref.child("device").child("3").update({"status": toggle_status3})
                    if name_device4 != "Chua duoc them vao":
                        if toggle_state4 is None:
                            toggle_status4 = "false"
                            device_ref = users_ref.child(user_id).child("device").child("4")
                            device_ref.update({
                                "status": toggle_status4
                            })
                            # user_ref = DataBase.child("Users").child(user.key())
                            # user_ref.child("device").child("4").update({"status": toggle_status4})
                        else:
                            toggle_status4 = toggle_state4
                            device_ref = users_ref.child(user_id).child("device").child("4")
                            device_ref.update({
                                "status": toggle_status4
                            })
                            # user_ref = DataBase.child("Users").child(user.key())
                            # user_ref.child("device").child("4").update({"status": toggle_status4})
                    if name_device5 != "Chua duoc them vao":
                        if toggle_state5 is None:
                            toggle_status5 = "false"
                            device_ref = users_ref.child(user_id).child("device").child("5")
                            device_ref.update({
                                "status": toggle_status5
                            })
                            # user_ref = DataBase.child("Users").child(user.key())
                            # user_ref.child("device").child("5").update({"status": toggle_status5})
                        else:
                            toggle_status5 = toggle_state5
                            device_ref = users_ref.child(user_id).child("device").child("5")
                            device_ref.update({
                                "status": toggle_status5
                            })
                    print("toggle0: ", toggle_status0)
                    print("toggle1: ", toggle_status1)
                    print("toggle2: ", toggle_status2)
                    print("toggle3: ", toggle_status3)
                    print("toggle4: ", toggle_status4)
                    print("toggle5: ", toggle_status5)
                # user_ref = DataBase.child("Users").child(user.key())
                # user_ref.child("device").child("5").update({"status": toggle_status5})
    # localid = user_info['users'][0]['localId']
    # print(localid)
    
    # print(data.child("localid").get("temperature",'N/A'))
    # user_ref = DataBase.child("Users").child(localid)
    # data2 =user_ref.child("device").child("0")
    # print(user_ref)
    # if data.each():
    #     current_data = {}
    #     for user in data.each():
    #         current_data[user.key()] = user.val()  # Lưu dữ liệu hiện tại
    #         # print(f"User ID: {user.key()}")  # In ra ID của người dùng
    #         if current_data[user.key()].get("email") == email:
    #             # Truy xuất thông tin DHT11 và các thiết bị
    #             dht11_data = user.val().get("DHT11", {})
    #             if dht11_data:
    #                 # print("DHT11 Data:")
    #                 # print(f"  - Do am: {dht11_data.get('doam', 'N/A')}")
    #                 humidity = dht11_data.get('doam', 'N/A')
                    
    #                 # print(f"  - Nhiet do: {dht11_data.get('nhietdo', 'N/A')}")
    #                 temperature = dht11_data.get('nhietdo', 'N/A')
                
    #             powerconsumtion = user.val().get("capacity", {})
                
    #             if powerconsumtion:
    #                 # print(f"  - Cong Suat Tieu Thu: {powerconsumtion.get('CongSuatTieuThu', 'N/A')}")

    #                 Total_Energy_Use = powerconsumtion.get('CongSuatTieuThu', 'N/A')
    
    # device = user.val().get("device", {})
    # user_ref = DataBase.child("Users").child(user.key())
    # print(user_ref)
                
    # device0_status = device[0]['status']
    # print(device0_status)
    
    
    
    
    
        
    return render(request, "dashboard2.html", {
                    'temp': temperature,
                    'capacity': Total_Energy_Use,
                    'namedv0': name_device0,
                    'namedv1': name_device1,
                    'namedv2': name_device2,
                    'namedv3': name_device3,
                    'namedv4': name_device4,
                    'namedv5': name_device5,
                    'toggles0': toggle_state0,
                    'toggles1': toggle_state1,
                    'toggles2': toggle_state2,
                    'toggles3': toggle_state3,
                    'toggles4': toggle_state4,
                    'toggles5': toggle_state5
    })