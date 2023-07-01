from django.contrib.auth.models import User  # make login / create account easier!
from django.contrib.postgres.fields import ArrayField
from django.db import models  # django's own model library!

"""
Put blank = True, at whatever field that is not required
"""


#My models starts here

class UserInformation(models.Model):
    plate_num = models.CharField(max_length = 15,blank=True)#this field is not a must!
    user = models.OneToOneField(User,on_delete = models.CASCADE,related_name = 'info')
    max_passengers = models.IntegerField(default=1,blank=True)#this field is not a must!
    special_vehicle_info = models.CharField(default = '',max_length=50,blank=True)#this field is not a must!
    is_driver = models.BooleanField(default=False)
    vehicle_type = models.CharField(max_length = 15,blank=True)#this field is not a must!
    def __str__(self):
        return self.user.username

class Vehicle(models.Model):
    make = models.CharField(max_length=20) # e.g. toyota
    is_new = models.BooleanField(default=True)
    year = models.IntegerField(default=-2022)
    is_full = models.BooleanField(default=False)
    type = models.CharField(max_length=30) # e.g. corolla



class Ride_order(models.Model):
    driver_id = models.IntegerField(default=-1, blank=True)
    is_open = models.CharField(max_length=20,default='open')#also can be 'confirmed' or 'completed'
    is_sharable = models.BooleanField(default=False)
    arrival_date = models.DateTimeField()
    rider_id = models.IntegerField(default=-1)
    destination = models.CharField(max_length=50)
    vehicle = models.CharField(max_length=15, blank=True)
    passenger_num = models.IntegerField()
    sharer_passengers = ArrayField(models.IntegerField(), default=[])
    sharer_id = ArrayField(models.IntegerField(), default=[])
    total_passenger_num = models.IntegerField(default=0)
    def __str__(self):
        return self.vehicle

    special_request = models.CharField(max_length=50, blank=True)
