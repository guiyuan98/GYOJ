from django.db import models
from django.utils import timezone

from account.models import User
from contest.models import Contest
from utils.models import JSONField
from utils.shortcuts import rand_str


class ExamSessionStatus:
    ACTIVE = "active"
    LOCKED = "locked"
    ENDED = "ended"


class ProctorEventType:
    START = "start"
    HEARTBEAT = "heartbeat"
    FOCUS_LOST = "focus_lost"
    CLIPBOARD = "clipboard"
    PROCESS = "process"
    NETWORK = "network"
    IP_CHANGED = "ip_changed"
    MACHINE_CHANGED = "machine_changed"
    VERSION_BLOCKED = "version_blocked"
    LOCKED = "locked"
    UNLOCKED = "unlocked"


class ExamPolicy(models.Model):
    contest = models.OneToOneField(Contest, on_delete=models.CASCADE, related_name="exam_policy")
    require_client = models.BooleanField(default=True)
    block_copy = models.BooleanField(default=True)
    block_switching = models.BooleanField(default=True)
    lock_ip = models.BooleanField(default=True)
    heartbeat_timeout_seconds = models.IntegerField(default=30)
    client_min_version = models.TextField(default="0.1.0")
    process_blacklist = JSONField(default=list)
    network_allowlist = JSONField(default=list)
    allow_local_samples = models.BooleanField(default=True)
    created_at = models.DateTimeField(auto_now_add=True)
    updated_at = models.DateTimeField(auto_now=True)

    class Meta:
        db_table = "exam_policy"

    def __str__(self):
        return f"ExamPolicy(contest={self.contest_id})"


class ExamSession(models.Model):
    session_key = models.TextField(default=rand_str, unique=True, db_index=True)
    contest = models.ForeignKey(Contest, on_delete=models.CASCADE, related_name="exam_sessions")
    user = models.ForeignKey(User, on_delete=models.CASCADE, related_name="exam_sessions")
    machine_fingerprint = models.TextField()
    locked_ip = models.TextField()
    client_version = models.TextField()
    status = models.TextField(default=ExamSessionStatus.ACTIVE, db_index=True)
    lock_reason = models.TextField(null=True, blank=True)
    last_heartbeat = models.DateTimeField(default=timezone.now)
    started_at = models.DateTimeField(auto_now_add=True)
    ended_at = models.DateTimeField(null=True, blank=True)
    unlock_count = models.IntegerField(default=0)
    extra = JSONField(default=dict)

    class Meta:
        db_table = "exam_session"
        ordering = ("-started_at",)

    def lock(self, reason: str):
        self.status = ExamSessionStatus.LOCKED
        self.lock_reason = reason
        self.save(update_fields=("status", "lock_reason"))

    def unlock(self):
        self.status = ExamSessionStatus.ACTIVE
        self.lock_reason = None
        self.unlock_count = models.F("unlock_count") + 1
        self.last_heartbeat = timezone.now()
        self.save(update_fields=("status", "lock_reason", "unlock_count", "last_heartbeat"))

    def __str__(self):
        return f"ExamSession(user={self.user_id}, contest={self.contest_id}, status={self.status})"


class ProctorEvent(models.Model):
    session = models.ForeignKey(ExamSession, on_delete=models.CASCADE, related_name="events")
    contest = models.ForeignKey(Contest, on_delete=models.CASCADE, related_name="proctor_events")
    user = models.ForeignKey(User, on_delete=models.CASCADE, related_name="proctor_events")
    event_type = models.TextField(db_index=True)
    severity = models.TextField(default="info")
    message = models.TextField(null=True, blank=True)
    payload = JSONField(default=dict)
    client_time = models.DateTimeField(null=True, blank=True)
    server_time = models.DateTimeField(default=timezone.now, db_index=True)
    created_by = models.ForeignKey(User, null=True, blank=True,
                                   on_delete=models.SET_NULL, related_name="created_proctor_events")

    class Meta:
        db_table = "proctor_event"
        ordering = ("-server_time",)

    def __str__(self):
        return f"{self.event_type}@{self.session_id}"
