# Generated by Django 2.2.26 on 2022-01-31 23:08

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('rides', '0008_vehicle_is_full_alter_ride_order_id_and_more'),
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
        migrations.AlterField(
            model_name='vehicle',
            name='id',
            field=models.AutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID'),
        ),
    ]
