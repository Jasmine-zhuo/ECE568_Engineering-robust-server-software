# product differentiation

## Functionalities

### Email notification

Users must register with an valid email address. After creating an account, there is an API for user to change their email address. Users will receive an email upon signing up, changing their email, changing their package's delivery destination and package delivered.


### Change delivery destination more easily

We provide a table view of all packages belonging to a user for them to make change of destination.

### Tracking history from source to destination

In our web side, we provide a page for users to view all their packages and all status. Users can view the warehouse id, warehouse location, pick up time, loaded time, and delivered time. We give users a whole view of information of their packages from the source to the destination. This is what the real UPS does.

### Multi Threads Handling tasks

We used multi-thread stategy to handle different incoming messages. This gives the system high efficient for concurrent tasks.

### Some UI and social function

We implement some UI design and attach some social media link at the bottom of homepage.

## Security

### Exceptions

We used try-catch block to protect the database and prevent different threads to race.

### Check permission when edit package destination

We enable everyone to search a package's detail with a Tracking Number, which is what the real UPS does. To maintain security, when user click 'edit destination' button, firstly, we check if user has logged in. If the user has logged in, then check if the owner of the package is same as the user. If the user has logged in, and the package belongs to the user, we will let the user change destination if the package has not been sent out.

### Password

We use Django Frame Work to maintain security, for user, their password will be encypted before being inserted into database, which will avoid hackers read the database and use raw password directly.
