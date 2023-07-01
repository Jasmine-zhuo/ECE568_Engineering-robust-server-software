from django import forms
from django.core.exceptions import ValidationError
from django.contrib.auth.forms import UserCreationForm
from django.contrib.auth.models import User
from .models import UserInformation, Ride_order


class RegisterAsUserForm(UserCreationForm):
    username = forms.CharField(label='Username', max_length=40)
    password = forms.CharField(label='Password', widget=forms.PasswordInput)
    email = forms.EmailField(label='Email')



class UserLoginForm(forms.Form):
    my_username = forms.CharField(label='my_username', max_length=20)
    def clean_my_username(self):
        my_username = self.cleaned_data.get('my_username')
        if len(my_username) >= 21:
            raise forms.ValidationError("Your user name is too long")
        return my_username

    my_password = forms.CharField(label='my_password', widget=forms.PasswordInput)
    def clean_my_password(self):
        my_password = self.cleaned_data.get('my_password')
        if len(my_password) >= 30:
            raise forms.ValidationError("Your user name is too long")
        return my_password

class RegisterAsDriverForm(forms.Form):
    vehicle_type = forms.CharField(label='Vehicle Type', max_length=15)
    def clean_vehicle_type(self):
        my_vehicle = self.cleaned_data.get('vehicle_type')
        if len(my_vehicle) >= 15:
            raise forms.ValidationError("Your vehicle type is too long!")
        return my_vehicle
    plate_num = forms.CharField(label='License Plate Number', max_length=15)

    def clean_plate_num(self):
        plate = self.cleaned_data.get('plate_num')
        if len(plate) == 0:
            raise forms.ValidationError("Your plate number cannot be empty.")
        return plate

    max_passengers = forms.IntegerField(label='Passenger Capacity')
    def clean_max_passengers(self):
        max_passengers = self.cleaned_data.get('max_passengers')
        if max_passengers >= 100:
            raise forms.ValidationError("Too many passengers!")
        return max_passengers

    special_vehicle_info = forms.CharField(label='Special Vehicle Info', max_length=50, widget=forms.Textarea,
                                           help_text=' (optional)',
                                           required=False)
    def clean_special_vehicle_info(self):
        special_vehicle_info = self.cleaned_data.get('special_vehicle_info')
        if len(special_vehicle_info) >= 50:
            raise forms.ValidationError("Too many special info!!")
        return special_vehicle_info


class RegisterAsSharerForm(forms.Form):

    passenger_num = forms.IntegerField(label='Number of Passengers')
    def clean_passenger_num(self):
        passenger_num = self.cleaned_data.get('passenger_num')
        if passenger_num >= 50:
            raise forms.ValidationError("Too many passengers!")
        return passenger_num



    early_arrival_date = forms.DateTimeField(label='Earliest Arrival Date&Time', help_text=' format: 2006-10-25 14:30')

    destination = forms.CharField(label='Destination', max_length=50)
    def clean_destination(self):
        destination  = self.cleaned_data.get('destination')
        if len(destination)  >= 30:
            raise forms.ValidationError("Too long!")
        return destination


    late_arrival_date = forms.DateTimeField(label='Latest Arrival Date&Time', help_text=' format: 2006-10-25 14:30')





class NewRideForm(forms.Form):
    sharable = forms.BooleanField(label='Willing to share this ride? (cannot change once submited)', required=False)
    arrival_date = forms.DateTimeField(label='Required Arrival Date&Time', help_text=' format: 2006-10-25 14:30')
    vehicle_type = forms.CharField(label='Vehicle Type', max_length=20, help_text=' (optional)', required=False)
    def clean_vehicle_type(self):
        my_vehicle = self.cleaned_data.get('vehicle_type')
        if len(my_vehicle) >= 15:
            raise forms.ValidationError("Your vehicle type is too long!")
        return my_vehicle


    passengers = forms.IntegerField(label='passengers')
    def clean_passengers(self):
        passengers = self.cleaned_data.get('passengers')
        if passengers >= 50:
            raise forms.ValidationError("Too many passengers!")
        return passengers

    destination = forms.CharField(label='Destination', max_length=30)
    def clean_destination(self):
        destination  = self.cleaned_data.get('destination')
        if len(destination)  >= 30:
            raise forms.ValidationError("Too long!")
        return destination

    special_vehicle_info = forms.CharField(label='Special Vehicle info', max_length=50, widget=forms.Textarea, help_text=' (optional)',
                              required=False)

    def clean_special_vehicle_info(self):
        special_vehicle_info = self.cleaned_data.get('special_vehicle_info')
        if len(special_vehicle_info) >= 50:
            raise forms.ValidationError("Too many special info!!")
        return special_vehicle_info


class Edit_ride_as_share_form(forms.Form):
    passenger = forms.IntegerField(label='Number of Passengers')