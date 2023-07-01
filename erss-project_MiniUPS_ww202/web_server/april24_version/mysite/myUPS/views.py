#
from django.shortcuts import render, redirect, get_object_or_404
from django.urls import reverse
from django.http import HttpResponse
from django.template import loader
from .models import *
from .forms import *
from django.contrib.auth import authenticate, login, logout
from django.contrib.auth.decorators import login_required
from django.contrib import messages
from django.core import mail


# Create your views here.

def index(request):
    template = loader.get_template('index.html')
    context = {
    }
    return HttpResponse(template.render(context, request))


#
def login_user(request):
    if request.user.is_active:
        return redirect('index')
    else:
        if request.method == "POST":
            username = request.POST['username']
            password = request.POST['password']
            user = authenticate(username=username, password=password)
            if user is not None:
                if user.is_active:
                    login(request, user)
                    return redirect('index')
                else:
                    return render(request, 'myUPS/login.html', {'error_message': 'Invalid Login'})
        return render(request, 'myUPS/login.html')


#
def register(request):
    form = RegisterForm(request.POST or None)
    if form.is_valid():
        user = form.save(commit=False)
        username = form.cleaned_data['username']
        password = form.cleaned_data['password']
        user.set_password(password)
        user.save()
        user = authenticate(username=username, password=password)
        account = Account()
        account.user = user
        world = World.objects.get(curr = True)
        world_id = world.world_id
        account.world = world
        account.save()
        if user is not None:
            if user.is_active:
                # Send emails
                subject = "%s, nice to meet you here!" % user.username 
                message = "Hi! We are happy to see you using our Mini-UPS service. This is the work of Hao Wu & Hongyang Li. Have a good day!"
                mail.send_mail(subject, message, 'mentos980822@gmail.com', [user.email])
                #
                #
                return redirect('login_user')
    context = {
        "form": form,
    }
    return render(request, 'myUPS/register.html', context)


#
@login_required
def all_pkg(request, u_id):
    user = request.user
    account = Account.objects.get(user_id = user.id)
    context = {
        'pkgs':Package.objects.filter(user_id = account.id)
    }
    return render(request,'myUPS/package.html',context)


#
def track_pkg(request):
    pack_id = request.POST.get('package_number')
    context = {
        'pkgs':Package.objects.filter(pk = pack_id), # pk is primary key
        'track_flag':True,
    }
    return render(request,'myUPS/package.html',context)


#
@login_required
def edit_email(request):
    form = EmailForm(request.POST or None)
    if form.is_valid():
        email_new = form.cleaned_data['email']
        user = request.user
        user.email = email_new
        user.save()
        # Send emails
        subject = "Message from Mini-UPS by Hao Wu & Hongyang Li." 
        message = "%s, this is a remind of you just changed your email address to this mail box." % user.username
        mail.send_mail(subject, message, 'mentos980822@gmail.com', [user.email])
        #
        return redirect('index')
    context = {
        "form": form,
    }
    return render(request, 'myUPS/edit_email.html', context)


#
@login_required
def logout_user(request):
    logout(request)
    return redirect('login_user')


#
@login_required
def track_edit_dest(request, p_id):
    pack_id = request.GET.get('package_id', '')
    #
    if pack_id == '':
        messages.warning(request, 'Please select a package!')
        context1 = {
            'pkgs':Package.objects.filter(pk = p_id),
            'track_flag': True,
        }
        return render(request,'myUPS/package.html',context1)
    
    #else:
    pack = Package.objects.get(pk = pack_id)
    print(request)

    if not request.user.is_active or not request.user.is_authenticated or pack.user == None or request.user.username != pack.user.user.username:
        messages.warning(request, 'You don\'t have access. Please log in as Owner!')
        logout_user(request)
        return redirect('login_user')

    context2 = {
        'pkgs':Package.objects.filter(pk = pack_id),
        'track_flag':True,
    }
    if pack.cur_status != "p":
        if pack.cur_status == "o":
            messages.warning(request, 'You can not change its destination! The package is already on its way!')
            return render(request, 'myUPS/package.html', context2) #???
        
        if pack.cur_status == "d":
            messages.warning(request, 'You can not change its destination! The package is already delivered!')
            return render(request, 'myUPS/package.html', context2) #???
        #
    #
    context3 = {
        'pkg':Package.objects.get(pk = pack_id),
        'track_flag':True,
    }
    return render(request,'myUPS/edit_dest.html', context3)


#
@login_required
def edit_dest(request):
    pack_id = request.GET.get('package_id', '')
    #
    if pack_id == '':
        messages.warning(request, 'Please select a package!')
        user = request.user
        account = Account.objects.get(user_id = user.id)
        context = {
            'pkgs':Package.objects.filter(user_id = account.id)
        }
        return render(request,'myUPS/package.html',context)

    #
    pack = Package.objects.get(pk = pack_id)
    print("---------------------------------")
    print(request)
    print("---------------------------------")

    if not request.user.is_active or not request.user.is_authenticated or pack.user == None or request.user.username != pack.user.user.username:
        messages.warning(request, 'You don\'t have access. Please log in as Owner!')
        logout_user(request)
        return redirect('login_user')

    if pack.cur_status != "p":
        if pack.cur_status == "o":
            messages.warning(request, 'You can not change its destination! The package is already on its way!')
            return redirect('all_pkg', u_id = request.user.id) #???

        if pack.cur_status == "d":
            messages.warning(request, 'You can not change its destination! The package is already delivered!')
            return redirect('all_pkg', u_id = request.user.id) #???
    #
    context = {
        'pkg':Package.objects.get(pk = pack_id)
    }
    return render(request,'myUPS/edit_dest.html', context)



#
@login_required
def save_dest(request):
    pack_id = request.POST.get('package_id', '')
    dest_x = request.POST.get('destination_x', '')
    dest_y = request.POST.get('destination_y', '')
    print(pack_id)
    print(dest_x)
    print(dest_y)
    pack = Package.objects.get(pk = pack_id)
    pack.dest_x = str(dest_x)
    pack.dest_y = str(dest_y)
    pack.save()
    #
    # Send emails
    subject = "Hi! %s." % request.user.username
    message = "This is a remind of you that you just changed the delivery address of your package. The Tracking Number is %s." % pack.package_id
    mail.send_mail(subject, message, 'mentos980822@gmail.com', [request.user.email])
    #
    #
    return redirect('all_pkg', u_id = request.user.id) # ???


#
@login_required
def loading(request):
    user = request.user
    account = Account.objects.get(user_id = user.id)
    context = {
        'pkgs':Package.objects.filter(user_id = account.id).filter(cur_status = 'p')
    }
    return render(request,'myUPS/package.html',context)


#
@login_required
def out(request):
    user = request.user
    account = Account.objects.get(user_id = user.id)
    context = {
        'pkgs':Package.objects.filter(user_id = account.id).filter(cur_status = 'o')
    }
    return render(request,'myUPS/package.html',context)


#
@login_required
def arrive(request):
    user = request.user
    account = Account.objects.get(user_id = user.id)
    context = {
        'pkgs':Package.objects.filter(user_id = account.id).filter(cur_status = 'd')
    }
    return render(request,'myUPS/package.html',context)


#
