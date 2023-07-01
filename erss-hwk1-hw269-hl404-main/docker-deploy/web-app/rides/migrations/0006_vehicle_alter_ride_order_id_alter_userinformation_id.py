# Generated by Django 4.0.1 on 2022-01-31 22:19

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('rides', '0005_auto_20220130_0402'),
    ]

    operations = [
        migrations.CreateModel(
            name='Vehicle',
            fields=[
                ('id', models.BigAutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID')),
                ('make', models.CharField(blank=True, max_length=15)),
                ('year', models.IntegerField(blank=True, default=2022)),
                ('type', models.CharField(blank=True, max_length=15)),
                ('is_new', models.BooleanField(default=True)),
            ],
        ),
        migrations.AlterField(
            model_name='ride_order',
            name='id',
            field=models.BigAutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID'),
        ),
        migrations.AlterField(
            model_name='userinformation',
            name='id',
            field=models.BigAutoField(auto_created=True, primary_key=True, serialize=False, verbose_name='ID'),
        ),
    ]
