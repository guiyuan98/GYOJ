from django.shortcuts import get_object_or_404
from django.utils import timezone

from account.decorators import login_required
from contest.models import Contest
from utils.api import APIView, validate_serializer

from .models import ExamPolicy, ExamSession, ExamSessionStatus, ProctorEvent, ProctorEventType
from .serializers import EventSerializer, HeartbeatSerializer, PolicySerializer, StartSessionSerializer, UnlockSerializer
from .services import client_ip_from_request, find_session_violations, is_version_allowed


DEFAULT_BLACKLIST = ["chrome.exe", "msedge.exe", "firefox.exe", "qq.exe", "wechat.exe", "devenv.exe"]


def serialize_policy(policy):
    return {
        "contest_id": policy.contest_id,
        "require_client": policy.require_client,
        "block_copy": policy.block_copy,
        "block_switching": policy.block_switching,
        "lock_ip": policy.lock_ip,
        "heartbeat_timeout_seconds": policy.heartbeat_timeout_seconds,
        "client_min_version": policy.client_min_version,
        "process_blacklist": policy.process_blacklist,
        "network_allowlist": policy.network_allowlist,
        "allow_local_samples": policy.allow_local_samples,
    }


def policy_for_contest(contest):
    policy, _ = ExamPolicy.objects.get_or_create(
        contest=contest,
        defaults={
            "process_blacklist": DEFAULT_BLACKLIST,
            "network_allowlist": [],
        },
    )
    return policy


def create_event(session, event_type, severity="info", message=None, payload=None, created_by=None):
    return ProctorEvent.objects.create(
        session=session,
        contest=session.contest,
        user=session.user,
        event_type=event_type,
        severity=severity,
        message=message,
        payload=payload or {},
        created_by=created_by,
    )


class ClientVersionAPI(APIView):
    def get(self, request):
        contest_id = request.GET.get("contest_id")
        if contest_id:
            policy = policy_for_contest(get_object_or_404(Contest, id=contest_id, visible=True))
            return self.success({"client_min_version": policy.client_min_version})
        return self.success({"client_min_version": "0.1.0"})


class ExamPolicyAPI(APIView):
    @login_required
    def get(self, request):
        contest_id = request.GET.get("contest_id")
        if not contest_id:
            return self.error("contest_id is required")
        contest = get_object_or_404(Contest, id=contest_id, visible=True)
        return self.success(serialize_policy(policy_for_contest(contest)))

    @login_required
    @validate_serializer(PolicySerializer)
    def put(self, request):
        data = request.data.copy()
        contest_id = data.pop("contest_id")
        contest = get_object_or_404(Contest, id=contest_id)
        if not request.user.is_contest_admin(contest):
            return self.error("No permission")
        policy = policy_for_contest(contest)
        for key, value in data.items():
            setattr(policy, key, value)
        policy.save()
        return self.success(serialize_policy(policy))


class StartSessionAPI(APIView):
    @login_required
    @validate_serializer(StartSessionSerializer)
    def post(self, request):
        data = request.data
        contest = get_object_or_404(Contest, id=data["contest_id"], visible=True)
        policy = policy_for_contest(contest)
        client_ip = client_ip_from_request(request)
        if not is_version_allowed(data["client_version"], policy.client_min_version):
            return self.error("Client version is too old")

        session = ExamSession.objects.create(
            contest=contest,
            user=request.user,
            machine_fingerprint=data["machine_fingerprint"],
            locked_ip=client_ip,
            client_version=data["client_version"],
            extra=data.get("extra") or {},
        )
        create_event(session, ProctorEventType.START, payload={"ip": client_ip, "client_version": data["client_version"]})
        return self.success({
            "session_key": session.session_key,
            "status": session.status,
            "locked_ip": session.locked_ip,
            "policy": serialize_policy(policy),
        })


class HeartbeatAPI(APIView):
    @login_required
    @validate_serializer(HeartbeatSerializer)
    def post(self, request):
        data = request.data
        try:
            session = ExamSession.objects.select_related("contest").get(session_key=data["session_key"], user=request.user)
        except ExamSession.DoesNotExist:
            return self.error("Session does not exist")

        policy = policy_for_contest(session.contest)
        client_ip = client_ip_from_request(request)
        violations = find_session_violations(policy, session, client_ip,
                                             data["machine_fingerprint"], data["client_version"])
        if violations:
            reason = ",".join(violations)
            session.lock(reason)
            create_event(session, ProctorEventType.LOCKED, severity="critical",
                         message=reason, payload={"ip": client_ip, "violations": violations})
        else:
            session.last_heartbeat = timezone.now()
            session.client_version = data["client_version"]
            session.save(update_fields=("last_heartbeat", "client_version"))

        return self.success({
            "status": session.status,
            "lock_reason": session.lock_reason,
            "server_time": timezone.now(),
            "policy": serialize_policy(policy),
        })


class ProctorEventAPI(APIView):
    @login_required
    @validate_serializer(EventSerializer)
    def post(self, request):
        data = request.data
        try:
            session = ExamSession.objects.select_related("contest").get(session_key=data["session_key"], user=request.user)
        except ExamSession.DoesNotExist:
            return self.error("Session does not exist")
        event = create_event(session, data["event_type"], data["severity"],
                             data.get("message"), data.get("payload") or {})
        if data.get("lock"):
            session.lock(data["event_type"])
            create_event(session, ProctorEventType.LOCKED, severity="critical", message=data["event_type"])
        return self.success({"event_id": event.id, "status": session.status, "lock_reason": session.lock_reason})


class UnlockSessionAPI(APIView):
    @login_required
    @validate_serializer(UnlockSerializer)
    def post(self, request):
        data = request.data
        try:
            session = ExamSession.objects.select_related("contest").get(session_key=data["session_key"])
        except ExamSession.DoesNotExist:
            return self.error("Session does not exist")
        if not request.user.is_contest_admin(session.contest):
            return self.error("No permission")
        session.unlock()
        create_event(session, ProctorEventType.UNLOCKED, severity="warning",
                     message=data.get("reason"), created_by=request.user)
        return self.success({"status": ExamSessionStatus.ACTIVE})


class AdminSessionListAPI(APIView):
    @login_required
    def get(self, request):
        contest_id = request.GET.get("contest_id")
        if not contest_id:
            return self.error("contest_id is required")
        contest = get_object_or_404(Contest, id=contest_id)
        if not request.user.is_contest_admin(contest):
            return self.error("No permission")
        qs = ExamSession.objects.filter(contest=contest).select_related("user")
        status = request.GET.get("status")
        if status:
            qs = qs.filter(status=status)
        data = self.paginate_data(request, qs)
        data["results"] = [{
            "session_key": item.session_key,
            "username": item.user.username,
            "status": item.status,
            "lock_reason": item.lock_reason,
            "locked_ip": item.locked_ip,
            "machine_fingerprint": item.machine_fingerprint,
            "client_version": item.client_version,
            "last_heartbeat": item.last_heartbeat,
            "unlock_count": item.unlock_count,
        } for item in data["results"]]
        return self.success(data)


class AdminEventListAPI(APIView):
    @login_required
    def get(self, request):
        contest_id = request.GET.get("contest_id")
        if not contest_id:
            return self.error("contest_id is required")
        contest = get_object_or_404(Contest, id=contest_id)
        if not request.user.is_contest_admin(contest):
            return self.error("No permission")
        qs = ProctorEvent.objects.filter(contest=contest).select_related("user", "session")
        event_type = request.GET.get("event_type")
        if event_type:
            qs = qs.filter(event_type=event_type)
        data = self.paginate_data(request, qs)
        data["results"] = [{
            "id": item.id,
            "session_key": item.session.session_key,
            "username": item.user.username,
            "event_type": item.event_type,
            "severity": item.severity,
            "message": item.message,
            "payload": item.payload,
            "server_time": item.server_time,
        } for item in data["results"]]
        return self.success(data)
