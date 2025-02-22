"""
URL configuration for smarthome_server project.

The `urlpatterns` list routes URLs to views. For more information please see:
    https://docs.djangoproject.com/en/4.2/topics/http/urls/
Examples:
Function views
    1. Add an import:  from my_app import views
    2. Add a URL to urlpatterns:  path('', views.home, name='home')
Class-based views
    1. Add an import:  from other_app.views import Home
    2. Add a URL to urlpatterns:  path('', Home.as_view(), name='home')
Including another URLconf
    1. Import the include() function: from django.urls import include, path
    2. Add a URL to urlpatterns:  path('blog/', include('blog.urls'))
"""
from django.contrib import admin
from django.urls import path
# from django.urls import path, include, re_path

# from django.views.generic.base import RedirectView
from django.conf import settings
from django.conf.urls.static import static

from smarthome_app import views

urlpatterns = [
    path('', views.dashboard, name="dashboard"),
    path('admin/', admin.site.urls),
    # path('', include('fire_dectection_web_app.urls')),
    path('dashboard/', views.dashboard, name = "dashboard"),
    path('login/', views.LoginPage, name="login"),
    path('logout/', views.LogoutPage, name='logout'),
] + static(settings.MEDIA_URL, document_root = settings.MEDIA_ROOT)
