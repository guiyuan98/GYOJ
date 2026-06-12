from django.contrib import admin

from .models import ExamPolicy, ExamSession, ProctorEvent


@admin.register(ExamPolicy)
class ExamPolicyAdmin(admin.ModelAdmin):
    list_display = ("contest", "require_client", "block_copy", "block_switching", "lock_ip", "client_min_version")


@admin.register(ExamSession)
class ExamSessionAdmin(admin.ModelAdmin):
    list_display = ("contest", "user", "status", "locked_ip", "client_version", "last_heartbeat")
    list_filter = ("status", "contest")
    search_fields = ("user__username", "machine_fingerprint", "locked_ip")


@admin.register(ProctorEvent)
class ProctorEventAdmin(admin.ModelAdmin):
    list_display = ("contest", "user", "event_type", "severity", "server_time")
    list_filter = ("event_type", "severity", "contest")
    search_fields = ("user__username", "message")

