from django.urls import path
from . import views, api

app_name = 'hosting'
urlpatterns = [
    path('', views.index, name='index'),
    path('signin/', views.signin, name='signin'),
    path('signout/', views.signout, name='signout'),
    path('edit/', views.edit, name='edit'),
    path('create/', views.create, name='create'),
    path('delete/', views.delete, name='delete'),
    path('rename/', views.rename, name='rename'),
    path('json/', api.get_json_data),
    path('list/', api.get_list),
    path('add/', api.add_testcase),
]
