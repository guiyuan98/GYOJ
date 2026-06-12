# OnlineJudge 服务端监考扩展

本目录提供一个可嵌入 QingdaoU OnlineJudge 后端的 Django 应用原型：

```text
server/onlinejudge_proctoring
```

它用于保存考试客户端的会话、策略和违规事件。

## 数据模型

- `ExamPolicy`：考试策略，例如是否必须使用客户端、是否禁止复制粘贴、心跳超时时间、客户端最低版本。
- `ExamSession`：一次学生考试会话，记录用户、比赛、公网 IP、机器指纹、客户端版本、状态和最近心跳。
- `ProctorEvent`：违规事件，例如切屏、复制粘贴、黑名单进程、IP 变化、机器变化。

## 接入方式

复制目录到 OnlineJudge 后端工程：

```text
OnlineJudge/onlinejudge_proctoring
```

在 Django settings 中加入：

```python
INSTALLED_APPS += ["onlinejudge_proctoring"]
```

挂载路由：

```python
url(r"^api/proctoring/", include("onlinejudge_proctoring.urls")),
```

执行迁移：

```bash
python3 manage.py makemigrations
python3 manage.py migrate
```

## 当前状态

这是毕业设计原型模块，已经包含基础模型、序列化器、服务函数、API 视图和 Django Admin 配置。后续需要根据实际 OnlineJudge 版本调整认证、权限和用户模型引用。
