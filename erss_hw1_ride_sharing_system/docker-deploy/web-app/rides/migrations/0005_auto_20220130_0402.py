# Generated by Django 2.2.26 on 2022-01-30 04:02

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('rides', '0004_rename_rider_order_id_ride_order_rider_id'),
    ]

    operations = [
        migrations.AlterField(
            model_name='ride_order',
            name='id',
            field=models.AutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID'),
        ),
        migrations.AlterField(
            model_name='userinformation',
            name='id',
            field=models.AutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID'),
        ),
    ]
