# ERSS-project-hw269-hl404
# Preparations:

## run the following commands:

sudo apt-get install gcc g++ make valgrind emacs git postgresql libpq-dev  python python3-pip libssl-dev libxerces-c-dev libpqxx-dev

sudo pip3 install django psycopg2


## Steps to run this mini-UPS program:

1. git clone all the files into your local repository

2. cd into 'web_server/april24_version/mysite' 

3. run:  python3 manage.py makemigrations 

4. run: python3 manage.py migrate

5. run: python3 manage.py flush

6. run: python3 manage.py runserver 0:8000

7. open a web browser, and the front-end website can be opened at vcm-xxxxx.vm.duke.edu:8000 (xxxxx is the VM No. that you use)

8. cd into 'backend_server' repository

9. use emacs to open 'main.cpp' and change the hostname/port num at line 104 & line 121

10. make sure the Amazon server and World server is already running (Implemented by other IG members)

11. run: make  and then run: ./miniUPS

12. Enjoy playing with our miniUPS website!
