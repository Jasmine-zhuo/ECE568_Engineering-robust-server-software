from django.urls import  path

from . import views
app_name = "rides" #this is the name space !

urlpatterns = [

path('login/',views.login,name='login'),
path('register_as_user/',views.register_as_user,name='register_as_user'),
path('',views.homepage,name='homepage'),
path('<int:id>/',views.all_my_info,name='all_my_info'),
path('logout/',views.logout,name='logout'),
path('<int:id>/register_as_driver/',views.register_as_driver,name='register_as_driver'),
path('<int:id>/signout_from_driver/',views.signout_from_driver,name='signout_from_driver'),
path('<int:id>/request_ride_as_rider/', views.request_ride_as_rider, name='request_ride_as_rider'),
path('<int:id>/<int:rid>/edit_ride_as_rider', views.edit_ride_as_rider, name='edit_ride_as_rider'),
path('rider_open_display/<int:id>',views.rider_open_display,name='rider_open_display'),
path('rider_confirmed_display/<int:id>',views.rider_confirmed_display,name='rider_confirmed_display'),
path('sharer_open_display/<int:id>',views.sharer_open_display,name='sharer_open_display'),
path('sharer_confirmed_display/<int:id>',views.sharer_confirmed_display,name='sharer_confirmed_display'),
path('driver_confirmed_display/<int:id>',views.driver_confirmed_display,name='driver_confirmed_display'),
path('<int:id>/edit_my_driver_info/', views.edit_my_driver_info, name='edit_my_driver_info'),
path('<int:id>/FindRideForDriver/',views.FindRideForDriver,name='FindRideForDriver'),
path('<int:id>/<int:rid>/cancel_a_ride/', views.cancel_a_ride, name='cancel_a_ride'),
path('<int:id>/<int:rid>/complete_a_ride/', views.complete_a_ride, name='complete_a_ride'),
path('<int:id>/request_ride_as_sharer/', views.request_ride_as_sharer, name='request_ride_as_sharer'),
path('<int:id>/<int:rid>/<int:passenger>/join_ride_as_sharer/', views.join_ride_as_sharer, name='join_ride_as_sharer'),
path('<int:id>/<int:rid>/delete_ride_as_sharer/', views.delete_ride_as_sharer, name='delete_ride_as_sharer'),
path('<int:id>/<int:rid>/edit_ride_as_sharer/', views.edit_ride_as_sharer, name='edit_ride_as_sharer'),
path('<int:id>/<int:rid>/confirm_a_drive',views.confirm_a_drive,name='confirm_a_drive'),

]