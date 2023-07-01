from django.urls import path
from . import views
from .models import *

urlpatterns = [
    path('', views.index, name='index'),
    path('register/', views.register , name = 'register'),
    path('login_user/', views.login_user, name='login_user'),
    path('package/',views.track_pkg, name = 'track_pkg'),
    path('package/<int:u_id>/',views.all_pkg, name = 'all_pkg'),
    path('edit_email/', views.edit_email , name = 'edit_email'),
    path('logout_user/', views.logout_user, name = 'logout_user'),

    path('edit_dest/',views.edit_dest, name = 'edit_dest'),
    path('save_dest/',views.save_dest, name = 'save_dest'),
    path('edit_dest/<p_id>', views.track_edit_dest, name = 'track_edit_dest'),
    
    path('loading/', views.loading, name = 'loading'),
    path('out/', views.out, name = 'out'),
    path('arrive/', views.arrive, name = 'arrive'),
    path('accounts/login/', views.login_user),
]
