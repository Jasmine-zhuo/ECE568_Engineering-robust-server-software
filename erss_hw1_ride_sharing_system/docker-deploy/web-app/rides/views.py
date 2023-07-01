#this has changed!
from django.contrib import messages
from django.http import Http404,HttpResponseRedirect
from django.shortcuts import render, get_object_or_404, redirect
from .models import UserInformation, Ride_order
from django.contrib.auth.models import User
from .forms import RegisterAsUserForm, UserLoginForm, RegisterAsDriverForm, NewRideForm, RegisterAsSharerForm, Edit_ride_as_share_form
from django.contrib.auth import authenticate
from django.contrib.auth.decorators import login_required
from django.contrib import auth
from django.urls import reverse

# Create your views here.

# view function1: Create a new account and register as a User!
def register_as_user(request):
    if request.method == 'GET':
        form = RegisterAsUserForm()
        message = ''
    else:
        form = RegisterAsUserForm(request.POST)
        if form.is_valid():
            user = User.objects.create_user( email=form.cleaned_data.get('email'),username=form.cleaned_data.get('username'),password= form.cleaned_data.get('password'),)
            user_info = UserInformation(user=user)
            is_driver = user_info.is_driver
            vehicle_type = user_info.vehicle_type
            plate_num = user_info.plate_num
            max_passengers = user_info.max_passengers
            special_vehicle_info = user_info.special_vehicle_info
            user_info.save()

            return redirect("/rides/login/")
        messages.error(request, "Unsuccessful registration. Invalid information!")
        message = 'invalid registration!!!'
    context = {'form': form, 'message': message}
    return render(request, 'rides/register_as_user.html', context)


# login to a already registered account!
def login(request):
    if request.method == 'GET':
        form = UserLoginForm()
    else:
        form = UserLoginForm(request.POST)
        if form.is_valid():
            usnm = form.cleaned_data.get('my_username')
            pswd = form.cleaned_data.get('my_password')
            user = authenticate(username=usnm, password=pswd)
            if user is not None:
                auth.login(request, user)
                messages.info(request, "You are now logged in as{usnm}!!")
                return HttpResponseRedirect(reverse('rides:all_my_info', args=[user.id]))
            else:
                return render(request, 'rides/login.html',
                              {'form': form, 'message': 'Invalid password! Please try again.'})

    return render(request, 'rides/login.html', {'form': form})


# display the homepage to users!!
def homepage(request):
    return render(request, 'rides/homepage.html')


# display all the informations for a user!
@login_required
def all_my_info(request, id):
    try:
        user = get_object_or_404(User, id=id)
        user_info = get_object_or_404(UserInformation, user=user)
    except User.DoesNotExist:
        raise Http404("User is not a registered user! Wrong ID!")
    user_information = get_object_or_404(UserInformation, user=user)
    context = {'user': user, 'user_info': user_information}
    return render(request, 'rides/all_my_info.html', context)


# log out from the website!
@login_required
def logout(request):
    auth.logout(request)
    messages.info(request, "You have successfully logged out.")
    return redirect("rides:homepage")


# A user register to become a driver!
@login_required
def register_as_driver(request, id):

    if request.method == 'GET':
        form = RegisterAsDriverForm()
        USER = get_object_or_404(User, id=id)
    else:  # a POST request!
        USER = get_object_or_404(User, id=id)
        form = RegisterAsDriverForm(request.POST)

        if form.is_valid():
            user_info = get_object_or_404(UserInformation, user=USER)
            try:
                user_info = get_object_or_404(UserInformation, user=USER)
            except user_info.DoesNotExist:
                raise Http404("user_info does not exits!")

            user_info = get_object_or_404(UserInformation, user=USER)

            new_user = User()
            new_driver = UserInformation( user=new_user,is_driver=True, max_passengers=form.cleaned_data['max_passengers'],
                                        vehicle_type=form.cleaned_data['vehicle_type'],plate_num=form.cleaned_data['plate_num'],
                                          special_vehicle_info= form.cleaned_data['special_vehicle_info'])
            user_info.is_driver = new_driver.is_driver
            user_info.max_passengers = new_driver.max_passengers
            user_info.vehicle_type = new_driver.vehicle_type
            user_info.plate_num = new_driver.plate_num
            user_info.special_vehicle_info = new_driver.special_vehicle_info
            user_info.save()
            return redirect(reverse('rides:all_my_info', args=[USER.id]))
    return render(request, 'rides/register_as_driver.html', {'form': form, 'user': USER})

@login_required
# what will happen if there are already orders ongoing?
def signout_from_driver(request, id):
    user = get_object_or_404(User, id=id)
    can_signout = False
    if user.is_driver:
        my_rides = Ride_order.objects.filter(driver_id=id)
        my_rides_list = list(my_rides)
        can_signout = my_rides_list.len() == 0
        if can_signout:  # otherwise we cannot signout!
            user.vehicle_type.all().delete()
            user.special_vehicle_info.all().delete()
            user.max_passengers.all().delete()
            user.is_driver = False
            user.save()
            return redirect(reverse("rides:signout_from_driver.html", args=[id]), {'can_signout': can_signout})
    # not a driver!
    else:
        messages.info(request, "You cannot signout now!!")
        return redirect(reverse("rides:signout_from_driver.html", args=[id]), {'can_signout': can_signout})


@login_required
# display the orders in which the user is the owner and the order is still open
def rider_open_display(request, id):
    user = get_object_or_404(User, id=id)
    try:
        user_info = get_object_or_404(UserInformation, user=user)
    except user_info.DoesNotExist:
        raise Http404("user_info does not exits!")
    user_info = get_object_or_404(UserInformation, user=user)
    result = list(Ride_order.objects.filter(is_open='open', rider_id=id))
    is_driver = user_info.is_driver
    vehicle_type = user_info.vehicle_type
    cur_open_rides_as_rider = []
    plate_num = user_info.plate_num
    max_passengers = user_info.max_passengers
    special_vehicle_info = user_info.special_vehicle_info
    for cur in result:
        num_of_groups = len(cur.sharer_id)
        for i in range(0, num_of_groups):
            date = cur.arrival_date
            passengers = cur.total_passenger_num
        cur_open_rides_as_rider.append(
            { 'arrival_date': cur.arrival_date,'group': len(cur.sharer_id),
              'destination': cur.destination,'id': cur.id,})

    return render(request, 'rides/rider_open_display.html', {
        'user': user,
        'user_profile': user_info,
        'openrides': cur_open_rides_as_rider,
    })


@login_required
def complete_a_ride(request, id, rid):
    try:
        order = get_object_or_404(Ride_order, id=id)
    except order.DoesNotExist:
        raise Http404("Order does not exits!")
    order = get_object_or_404(Ride_order, id=rid)
    order.is_open = 'completed'
    order.save()
    return redirect(reverse('rides:all_my_info', args=[id]))








# display the orders in which the user is the owner and the order is already confirmed by a driver
@login_required
def rider_confirmed_display(request, id):
    try:
        user = get_object_or_404(User, id=id)
    except user.DoesNotExist:
        raise Http404("user_info does not exits!")

    user = get_object_or_404(User, id=id)
    try:
        user_info = get_object_or_404(UserInformation, user=user)
    except user_info.DoesNotExist:
        raise Http404("user_info does not exits!")
    user_info = get_object_or_404(UserInformation, user=user)

    result =list(Ride_order.objects.filter(is_open='confirmed', rider_id=id))
    confimed_rides=[]
    is_driver = user_info.is_driver
    vehicle_type = user_info.vehicle_type
    plate_num = user_info.plate_num
    max_passengers = user_info.max_passengers
    special_vehicle_info = user_info.special_vehicle_info
    for i in range(0, len(result)):
        # get driver info
        cur_d_id = result[i].driver_id
        cur_driver = get_object_or_404(User, id=cur_d_id)
        temp_sharers = []
        for j in range(0, len(result[i].sharer_id)):

            temp_sharers.append({'name': get_object_or_404(User, id=result[i].sharer_id[j]).username, '# of passengers': result[i].sharer_passengers[j]})
        confimed_rides.append(
            {
                'vehicle_info': get_object_or_404(UserInformation, user=cur_driver).vehicle_type,
                'vehicle_plate': get_object_or_404(UserInformation, user=cur_driver).plate_num, 'sharers': temp_sharers,
                'destination': result[i].destination, 'arrivaldate': result[i].arrival_date, 'driver_name': cur_driver.username,
             'id': result[i].id,
             'group': len(result[i].sharer_id)})

    return render(request, 'rides/rider_confirmed_display.html',
                  {'user': user, 'user_profile': user_info, 'cfm_info_rider': confimed_rides})


@login_required
# display the orders in which the user is the sharer and the order is still open
def sharer_open_display(request, id):
    open_order_as_sharer = []
    user = get_object_or_404(User, id=id)
    user_info = get_object_or_404(UserInformation, user=user)
    result = list(Ride_order.objects.filter(is_open='open'))
    try:
        user_info = get_object_or_404(UserInformation, user=user)
    except user_info.DoesNotExist:
        raise Http404("user_info does not exits!")
    for cur in result:
        if id in cur.sharer_id:
            open_order_as_sharer.append(cur)

    return render(request, 'rides/sharer_open_display.html', {
        'user': user,
        'user_profile': user_info,
        'openshares': open_order_as_sharer}
    )


@login_required
# display the orders in which the user is the sharer and the order is already confirmed by a driver
def sharer_confirmed_display(request, id):
    order_confirmed_as_sharer = []
    user = get_object_or_404(User, id=id)
    user_info = get_object_or_404(UserInformation, user=user)

    result = list(Ride_order.objects.filter(is_open='confirmed'))
    try:
        user_info = get_object_or_404(UserInformation, user=user)
    except user_info.DoesNotExist:
        raise Http404("user_info does not exits!")
    for cur in result:
        if id in cur.sharer_id:
            curt_driver = get_object_or_404(User, id=cur.driver_id)
            order_confirmed_as_sharer.append(
                {
                 'destination': cur.destination, 'arrival_date': cur.arrival_date,
                 'driver_username': curt_driver.username,
                 'vehicle_info': get_object_or_404(UserInformation, user=get_object_or_404(User, id=cur.driver_id)).vehicle_type,
                 'vehicle_plate': get_object_or_404(UserInformation, user=curt_driver).plate_num,
                'rider_username': get_object_or_404(User, id=cur.rider_id).username})

    return render(request, 'rides/sharer_confirmed_display.html', {'user': user,
                                                                   'user_profile': user_info,
                                                                   'confirmedshares':     order_confirmed_as_sharer })


@login_required
# display the orders in which the user is the driver and the order is already confirmed by him/her
def driver_confirmed_display(request, id):
    rider=''
    user = get_object_or_404(User, id=id)
    user_info = get_object_or_404(UserInformation, user=user)
    try:
        user_info = get_object_or_404(UserInformation, user=user)
    except user_info.DoesNotExist:
        raise Http404("user_info does not exits!")
    user_info = get_object_or_404(UserInformation, user=user)
    result = list(Ride_order.objects.filter(is_open='confirmed', driver_id=id))
    order=''
    sharer_list = []
    cur_order = None
    if len(result) > 0:
          cur_order = result[0]
          for i in range(0, len(cur_order.sharer_id)):
            cur_name = get_object_or_404(User, id=cur_order.sharer_id[i]).username
            curt_sharer = {'name': cur_name, 'num': cur_order.sharer_passengers[i]}
            sharer_list.append(curt_sharer)
            rider=get_object_or_404(User, id=cur_order.rider_id).username

    return render(request, 'rides/driver_confirmed_display.html', {'user': user,
                                                                   'user_profile': user_info,
                                                                   'has_drive': len(result) > 0,
                                                                   'drive': cur_order,
                                                                   'owner': rider,
                                                                   'sharers': sharer_list})



@login_required
def request_ride_as_rider(request, id):
    user = get_object_or_404(User, id=id)
    if request.method == 'GET':
        form = NewRideForm()
    else:
        form = NewRideForm(request.POST)
        if form.is_valid():
            ride = Ride_order(destination=form.cleaned_data['destination'], arrival_date=form.cleaned_data['arrival_date'], passenger_num=form.cleaned_data['passengers'],
                              is_sharable=form.cleaned_data['sharable'],
                              vehicle=form.cleaned_data['vehicle_type'], special_request=form.cleaned_data['special_vehicle_info'], rider_id=id)
            ride.save()

            return redirect(reverse("rides:all_my_info", args=[id]))

    return render(request, 'rides/request_ride_as_rider.html', {'form': form, 'user': user})



@login_required
def cancel_a_ride(request, id, rid):
    try:
        order = get_object_or_404(Ride_order, id=id)
    except order.DoesNotExist:
        raise Http404("Order does not exits!")
    get_object_or_404(Ride_order, id=rid).delete()
    return redirect(reverse('rides:all_my_info', args=[id]))






# edit current ride for rider
@login_required
def edit_ride_as_rider(request, id, rid):
    try:
        user = get_object_or_404(User, id=id)
    except user.DoesNotExist:
        raise Http404("user_info does not exits!")
    user = get_object_or_404(User, id=id)
    try:
        user_info = get_object_or_404(UserInformation, user=user)
    except user_info.DoesNotExist:
        raise Http404("user_info does not exits!")
    user_info = get_object_or_404(UserInformation, user=user)
    order= get_object_or_404(Ride_order, id=rid)

    if order.is_open == 'confirmed' or order.is_open == 'completed':
            redirect(reverse('users:all_my_info', args=[id]))

    else:
        if request.method == 'GET':
            form = NewRideForm({
                'destination': order.destination,
                'arrivaldate': order.arrival_date,
                'passenger': order.passenger_num,
                'vehicle': order.vehicle,
                'special': order.special_request
            })
        else:
            form = NewRideForm(request.POST)
            if form.is_valid():
                # status default open
                new_user = User()
                new_order = Ride_order(destination=form.cleaned_data['destination'],arrival_date=form.cleaned_data['arrival_date'],
                                            passenger_num=form.cleaned_data['passengers'],vehicle = form.cleaned_data['vehicle_type'],
                                       special_request=form.cleaned_data['special_vehicle_info'] )
                order.destination = new_order.destination
                order.arrival_date = new_order.arrival_date
                order.passenger_num = new_order.passenger_num
                order.vehicle = new_order.vehicle
                order.special_request = new_order.special_request

                order.save()
                return redirect(reverse('rides:all_my_info', args=[user.id]))
        return render(request, 'rides/edit_ride_as_rider.html',
                          {'user': user, 'user_profile': user_info,
                           'form': form})


# edit driver info
def edit_my_driver_info(request, id):
    try:
        user = get_object_or_404(User, id=id)
    except user.DoesNotExist:
        raise Http404("user_info does not exits!")
    user = get_object_or_404(User, id=id)
    try:
        user_info = get_object_or_404(UserInformation, user=user)
    except user_info.DoesNotExist:
        raise Http404("user_info does not exits!")
    user_info = get_object_or_404(UserInformation, user=user)

    if request.method == 'GET':
        messages.success(request,'The method is Get!')
        form = RegisterAsDriverForm({
            'max_passengers': user_info.max_passengers,
            'special_vehicle_info': user_info.special_vehicle_info,
            'vehicle_type': user_info.vehicle_type,
            'plate_num': user_info.plate_num,
        })
        return render(request, 'rides/edit_my_driver_info.html',
                      {'form': form,'user': user, 'user_profile': user_info})

    else:  # request.method == 'POST':

        has_drive = list(Ride_order.objects.filter(is_open='confirmed', driver_id=id))
        if len(has_drive) > 0:
            messages.success(request,'You have confirmed orders as a driver! ')
            return redirect(reverse('rides:all_my_info', args=[user.id]))

        form = RegisterAsDriverForm(request.POST)
        if form.is_valid():
            new_user=User()
            plate = form.cleaned_data['plate_num']
            new_info=UserInformation(user=new_user,
                                     vehicle_type=form.cleaned_data['vehicle_type'],
                                     plate_num=plate,
                                     max_passengers=form.cleaned_data['max_passengers'],
                                     special_vehicle_info=form.cleaned_data['special_vehicle_info']
                                )
            user_info.max_passengers = new_info.max_passengers
            user_info.special_vehicle_info = new_info.special_vehicle_info
            user_info.vehicle_type = new_info.vehicle_type
            user_info.plate_num = new_info.plate_num
            user_info.save()
        return redirect(reverse('rides:all_my_info', args=[user.id]))








# handle ride for new join sharer
@login_required
def join_ride_as_sharer(request, id, rid, passenger):
    try:
        order = get_object_or_404(Ride_order, id=id)
    except order.DoesNotExist:
        raise Http404("Order does not exits!")
    order = get_object_or_404(Ride_order, id=rid)
    if order.is_open == 'open':
        order.sharer_passengers.append(passenger)
        order.total_passenger_num += passenger
        order.sharer_id.append(id)
        order.save()

    return redirect(reverse('rides:all_my_info', args=[id]))

@login_required
def edit_ride_as_sharer(request, id, rid):
    try:
        user = get_object_or_404(User, id=id)
    except user.DoesNotExist:
        raise Http404("user_info does not exits!")
    user = get_object_or_404(User, id=id)
    try:
        user_info = get_object_or_404(UserInformation, user=user)
    except user_info.DoesNotExist:
        raise Http404("user_info does not exits!")
    user_info = get_object_or_404(UserInformation, user=user)

    if request.method == 'GET':
        form = Edit_ride_as_share_form()
        return render(request, 'rides/edit_ride_as_sharer.html', {'form': form, 'user': user})

    else:  # request.method == 'POST':
        form = Edit_ride_as_share_form(request.POST)
        if form.is_valid():
            new_order=Ride_order(passenger_num=form.cleaned_data['passenger'],
                                 )
            try:
                order = get_object_or_404(Ride_order, id=id)
            except order.DoesNotExist:
                raise Http404("Order does not exits!")
            order = get_object_or_404(Ride_order, id=rid)
            order.sharer_passengers[order.sharer_id.index(id)] = new_order.passenger_num
            order.save()
            return redirect(reverse('rides:all_my_info', args=[user.id]))


# find available rides for driver
@login_required
def FindRideForDriver(request, id):

    try:
        user = get_object_or_404(User, id=id)
    except user.DoesNotExist:
        raise Http404("user_info does not exits!")
    user = get_object_or_404(User, id=id)
    try:
        user_info = get_object_or_404(UserInformation, user=user)
    except user_info.DoesNotExist:
        raise Http404("user_info does not exits!")
    user_info = get_object_or_404(UserInformation, user=user)
    result = list(Ride_order.objects.filter(is_open='open'))
    pickable_rides = []
    # condition for passenger limit&sharer
    for ride in result:
        # continue if driver's sharer
        if id in ride.sharer_id:  # cannot take the order where the driver is a  sharer himself!
            continue
        if id == ride.rider_id:  # cannot take the order where the driver is a rider himeself!
            continue
        if ride.vehicle != '' and ride.vehicle != user_info.vehicle_type:  # The order has a vihicle requirement but the driver cannot meet!
            continue
        if ride.special_request != '' and ride.special_request != user_info.special_vehicle_info:  # the order has special request but the driver cannot meet
            continue

        pickable_rides.append(ride)
    return render(request, 'rides/FindRideForDriver.html', {'user': user, 'pickable_rides': pickable_rides})


# A driver can confirm an order if he is interested!
@login_required
def confirm_a_drive(request, id, rid):
    order = get_object_or_404(Ride_order, id=id)
    driver = get_object_or_404(User, id=id)
    try:
        order = get_object_or_404(Ride_order, id=id)
    except order.DoesNotExist:
        raise Http404("Order does not exits!")
    order = get_object_or_404(Ride_order, id=rid)
    order.driver_id = id
    order.is_open = 'confirmed'
    order.save()
    return redirect(reverse('rides:all_my_info', args=[id]))




@login_required
def request_ride_as_sharer(request, id):
    try:
        user = get_object_or_404(User, id=id)
    except user.DoesNotExist:
        raise Http404("user_info does not exits!")
    user = get_object_or_404(User, id=id)
    try:
        user_info = get_object_or_404(UserInformation, user=user)
    except user_info.DoesNotExist:
        raise Http404("user_info does not exits!")
    user_info = get_object_or_404(UserInformation, user=user)

    if request.method == 'GET':
        form = RegisterAsSharerForm()
    else:  # request.method == 'POST':
        form = RegisterAsSharerForm(request.POST)
        if form.is_valid():
            order_as_sharer = []
            ride_order_result =list( Ride_order.objects.filter(is_open='open', destination = form.cleaned_data['destination'], is_sharable = True))

            for cur in ride_order_result:
                time_is_ok = (form.cleaned_data['early_arrival_date'] <= cur.arrival_date and form.cleaned_data['late_arrival_date'] >= cur.arrival_date)
                if  time_is_ok and id not in cur.sharer_id:
                    order_as_sharer.append(cur)

            return render(request, 'rides/result_of_searching_as_sharer.html',
                          {'user': user, 'sharerides': order_as_sharer, 'passenger': form.cleaned_data['passenger_num']})

    return render(request, 'rides/request_ride_as_sharer.html', {'form': form, 'user': user})


@login_required
def delete_ride_as_sharer(request, id, rid):
    try:
        order = get_object_or_404(Ride_order, id=id)
    except order.DoesNotExist:
        raise Http404("Order does not exits!")
    order = get_object_or_404(Ride_order, id=rid)
    i = order.sharer_id.index(id)
    del order.sharer_passengers[i]
    order.sharer_id.remove(id)
    order.save()
    return redirect(reverse('rides:all_my_info', args=[id]))
