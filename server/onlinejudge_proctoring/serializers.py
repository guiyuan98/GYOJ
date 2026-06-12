from utils.api import serializers


class PolicySerializer(serializers.Serializer):
    contest_id = serializers.IntegerField()
    require_client = serializers.BooleanField()
    block_copy = serializers.BooleanField()
    block_switching = serializers.BooleanField()
    lock_ip = serializers.BooleanField()
    heartbeat_timeout_seconds = serializers.IntegerField(min_value=5, max_value=3600)
    client_min_version = serializers.CharField(max_length=32, allow_blank=True)
    process_blacklist = serializers.ListField(child=serializers.CharField(max_length=128), allow_empty=True)
    network_allowlist = serializers.ListField(child=serializers.CharField(max_length=256), allow_empty=True)
    allow_local_samples = serializers.BooleanField()


class StartSessionSerializer(serializers.Serializer):
    contest_id = serializers.IntegerField()
    machine_fingerprint = serializers.CharField(max_length=256)
    client_version = serializers.CharField(max_length=32)
    extra = serializers.DictField(required=False)


class HeartbeatSerializer(serializers.Serializer):
    session_key = serializers.CharField(max_length=128)
    machine_fingerprint = serializers.CharField(max_length=256)
    client_version = serializers.CharField(max_length=32)
    metrics = serializers.DictField(required=False)


class EventSerializer(serializers.Serializer):
    session_key = serializers.CharField(max_length=128)
    event_type = serializers.CharField(max_length=64)
    severity = serializers.ChoiceField(choices=["info", "warning", "critical"], default="warning")
    message = serializers.CharField(required=False, allow_blank=True, allow_null=True)
    payload = serializers.DictField(required=False)
    client_time = serializers.DateTimeField(required=False, allow_null=True)
    lock = serializers.BooleanField(required=False, default=False)


class UnlockSerializer(serializers.Serializer):
    session_key = serializers.CharField(max_length=128)
    reason = serializers.CharField(max_length=512, required=False, allow_blank=True)

