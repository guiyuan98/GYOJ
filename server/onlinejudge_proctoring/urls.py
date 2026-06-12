from django.conf.urls import url

from .views import (AdminEventListAPI, AdminSessionListAPI, ClientVersionAPI, ExamPolicyAPI, HeartbeatAPI,
                    ProctorEventAPI, StartSessionAPI, UnlockSessionAPI)


urlpatterns = [
    url(r"^version/?$", ClientVersionAPI.as_view(), name="proctoring-version"),
    url(r"^policy/?$", ExamPolicyAPI.as_view(), name="proctoring-policy"),
    url(r"^session/start/?$", StartSessionAPI.as_view(), name="proctoring-session-start"),
    url(r"^session/heartbeat/?$", HeartbeatAPI.as_view(), name="proctoring-session-heartbeat"),
    url(r"^session/unlock/?$", UnlockSessionAPI.as_view(), name="proctoring-session-unlock"),
    url(r"^event/?$", ProctorEventAPI.as_view(), name="proctoring-event"),
    url(r"^admin/sessions/?$", AdminSessionListAPI.as_view(), name="proctoring-admin-sessions"),
    url(r"^admin/events/?$", AdminEventListAPI.as_view(), name="proctoring-admin-events"),
]
