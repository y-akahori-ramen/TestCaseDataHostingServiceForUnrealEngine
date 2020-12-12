from django.http import HttpResponse, HttpResponseRedirect, HttpResponseNotFound
from django.shortcuts import render, get_object_or_404
from django.urls import reverse
from django.contrib.auth.decorators import login_required
from django.contrib.auth import authenticate, login, logout

from .models import TestCase
from .testcase import view_util
from .testcase.data import validate_new_name, validate_rename_name


def signin(request):
    if request.method == 'GET':
        if request.user.is_authenticated:
            return HttpResponseRedirect(reverse('hosting:index'))
        else:
            return render(request, 'hosting/signin.html')
    elif request.method == 'POST':
        username = request.POST['inputUser']
        userpassword = request.POST['inputPassword']
        user = authenticate(request, username=username, password=userpassword)

        if user is not None:
            login(request, user=user)
            # ログインしていない状態でページにアクセスした場合このビューが表示され、その場合はnextに戻り先が入っている
            # nextが存在すればそちらへ移動し、nextがなければトップページに移動
            if 'next' in request.GET:
                return HttpResponseRedirect(request.GET['next'])
            else:
                return HttpResponseRedirect(reverse('hosting:index'))
        else:
            return render(request, 'hosting/signin.html')
    else:
        return render(request, 'hosting/signin.html')


def signout(request):
    logout(request)
    return HttpResponseRedirect(reverse('hosting:signin'))


@login_required(login_url='/signin')
def index(request):
    if 'path' in request.GET:
        cur_dir = request.GET['path']
    else:
        cur_dir = ''

    list_items = view_util.get_testcase_list_items(cur_dir)
    breadcrumb_items = view_util.get_breadcrumb_items(cur_dir)
    params = {
        'list_items': list_items,
        'breadcrumb_items': breadcrumb_items,
        'create_initial_testcasename': cur_dir + '/',
    }

    return render(request, 'hosting/listpage.html', params)


@login_required(login_url='/signin')
def edit_get(request):
    if 'path' in request.GET:
        path = request.GET['path']
    else:
        path = ''

    # submitされた場合にデータの更新を行い、以前のリストページへリダイレクト

    testcase_data = get_object_or_404(TestCase, title_path=path)

    breadcrumb_items = view_util.get_breadcrumb_items(path)

    # パンくずリストの末尾が自分のデータのパスなのでそれより１つ前が戻るべきパスとなる
    backurl_path = breadcrumb_items[-2].url_param

    params = {
        'breadcrumb_items': breadcrumb_items,
        'summary': testcase_data.summary,
        'data': testcase_data.testcase_data,
        'backurl_path': backurl_path,
        'cur_path': path,
    }
    return render(request, 'hosting/editpage.html', params)


def edit_post(request):
    testcase_data = get_object_or_404(TestCase, title_path=request.GET['path'])
    testcase_data.summary = request.POST['summary']
    testcase_data.testcase_data = request.POST['testcasedata']
    testcase_data.save()

    # 一つ前のリストに戻す
    breadcrumb_items = view_util.get_breadcrumb_items(request.GET['path'])
    backurl_path = breadcrumb_items[-2].url_param

    redirect_uri = reverse('hosting:index') + f'?path={backurl_path}'
    return HttpResponseRedirect(redirect_uri)


@login_required(login_url='/signin')
def edit(request):
    if request.method == 'GET':
        return edit_get(request)
    elif request.method == 'POST':
        return edit_post(request)


@login_required(login_url='/signin')
def create(request):
    if request.method == 'POST':
        new_name = request.POST['testcaseName']
        validate_result = validate_new_name(new_name)
        if validate_result.is_valid:
            new_testcase = TestCase(title_path=new_name)
            new_testcase.save()
            redirect_uri = reverse('hosting:edit') + f'?path={new_name}'
            return HttpResponseRedirect(redirect_uri)
        else:
            params = {
                'error_msg': validate_result.desc,
            }
            return render(request, 'hosting/errorpage.html', params)
    else:
        return HttpResponseNotFound()


@login_required(login_url='/signin')
def delete(request):
    if request.method == 'POST':
        delete_name = request.POST['testcaseName']
        testcase_data = get_object_or_404(TestCase, title_path=delete_name)
        testcase_data.delete()
        return HttpResponseRedirect(reverse('hosting:index'))
    else:
        return HttpResponseNotFound()


@login_required(login_url='/signin')
def rename(request):
    if request.method == 'POST':
        new_name = request.POST['testcaseName']
        old_name = request.POST['oldName']
        validate_result = validate_rename_name(new_name, old_name)
        if validate_result.is_valid:
            testcase_data = get_object_or_404(TestCase, title_path=old_name)
            testcase_data.title_path = new_name
            testcase_data.save()
            return HttpResponseRedirect(reverse('hosting:index'))
        else:
            params = {
                'error_msg': validate_result.desc,
            }
            return render(request, 'hosting/errorpage.html', params)

    else:
        return HttpResponseNotFound()
