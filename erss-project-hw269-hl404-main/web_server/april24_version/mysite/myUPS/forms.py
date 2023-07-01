from django import forms
from django.contrib.auth.models import User

from .models import Package

#
class RegisterForm(forms.ModelForm):

    password = forms.CharField(widget=forms.PasswordInput)
    
    class Meta:
        model = User
        fields = ['username', 'password', 'email']



#
class EditDesForm(forms.ModelForm):

    dest_x = forms.CharField(min_length=1)
    dest_y = forms.CharField(min_length=1)
    
    class Meta:
        model = Package
        fields = ['dest_x', 'dest_y']



#
class EmailForm(forms.ModelForm):

    email = forms.CharField(min_length=1)
    
    class Meta:
        model = User
        fields = ['email']




