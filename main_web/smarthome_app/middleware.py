# middleware.py
import pyrebase
from django.utils.deprecation import MiddlewareMixin
from django.http import HttpResponse
from django.contrib.auth import get_user_model

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

firebase = pyrebase.initialize_app(config)
authe = firebase.auth()

class FirebaseAuthMiddleware(MiddlewareMixin):
    def process_request(self, request):
        # test_value = request.session.get('uid')
        # print(test_value)
        # return HttpResponse(f"Session Value: {test_value}")
        id_token = request.COOKIES.get('idToken')
        # id_token = request.session.get('uid')
        # print(id_token)
        if id_token:
            try:
            #    idtoken = request.session['uid']
                user_info = authe.get_account_info(id_token)
                # print("user_info", user_info)
                if user_info:
                    user_data = user_info['users'][0]
                    print("user_data", user_data)
                    email_verified = user_info['users'][0]['emailVerified']
                    print("email_verified", email_verified)
                    email = user_data['email']
                    print("email", email)
                    # user = get_user_model().objects.get(email=email)
                    request.user = email
            except Exception as e:
                print(f"Error verifying token: {e}")
                request.user = None  # Invalidate user if token is invalid
        else:
            request.user = None  # If no token, no user
            
# class FirebaseAuthentication(authentication.BaseAuthentication):
#     def authenticate(self, request):

#         token = request.headers.get('Authorization')
#         if not token:
#             print("Token not found")
#             return None

#         try:
#             decoded_token = auth.verify_id_token(token)
#             uid = decoded_token["uid"]
#             print(uid)
#         except:
#             return None
            
#         try:
#             user = User.objects.get(username=uid)
#             return user

#         except ObjectDoesNotExist:
#             return None