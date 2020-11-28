from django.urls import path
from . import views

app_name = 'hosting'
urlpatterns = [
    path('', views.index, name='index'),
    path('signin/', views.signin, name='signin'),
    path('edit/', views.edit, name='edit'),
    path('create/', views.create, name='create'),
    path('delete/', views.delete, name='delete'),
    path('rename/', views.rename, name='rename'),
    path('json/', views.get_json_data, name='json'),
]
