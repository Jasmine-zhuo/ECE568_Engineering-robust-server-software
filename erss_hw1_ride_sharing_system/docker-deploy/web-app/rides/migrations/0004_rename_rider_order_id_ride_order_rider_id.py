# Generated by Django 4.0.1 on 2022-01-28 20:18

from django.db import migrations


class Migration(migrations.Migration):

    dependencies = [
        ('rides', '0003_ride_order_userinformation_delete_choice_and_more'),
    ]

    operations = [
        migrations.RenameField(
            model_name='ride_order',
            old_name='rider_order_id',
            new_name='rider_id',
        ),
    ]