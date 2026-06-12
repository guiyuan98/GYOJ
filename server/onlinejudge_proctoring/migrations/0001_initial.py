from django.db import migrations, models
import django.utils.timezone
import django.db.models.deletion
import utils.models
import utils.shortcuts


class Migration(migrations.Migration):

    initial = True

    dependencies = [
        ("account", "0012_userprofile_language"),
        ("contest", "0010_auto_20190326_0201"),
    ]

    operations = [
        migrations.CreateModel(
            name="ExamPolicy",
            fields=[
                ("id", models.AutoField(auto_created=True, primary_key=True, serialize=False, verbose_name="ID")),
                ("require_client", models.BooleanField(default=True)),
                ("block_copy", models.BooleanField(default=True)),
                ("block_switching", models.BooleanField(default=True)),
                ("lock_ip", models.BooleanField(default=True)),
                ("heartbeat_timeout_seconds", models.IntegerField(default=30)),
                ("client_min_version", models.TextField(default="0.1.0")),
                ("process_blacklist", utils.models.JSONField(default=list)),
                ("network_allowlist", utils.models.JSONField(default=list)),
                ("allow_local_samples", models.BooleanField(default=True)),
                ("created_at", models.DateTimeField(auto_now_add=True)),
                ("updated_at", models.DateTimeField(auto_now=True)),
                ("contest", models.OneToOneField(on_delete=django.db.models.deletion.CASCADE, related_name="exam_policy", to="contest.Contest")),
            ],
            options={"db_table": "exam_policy"},
        ),
        migrations.CreateModel(
            name="ExamSession",
            fields=[
                ("id", models.AutoField(auto_created=True, primary_key=True, serialize=False, verbose_name="ID")),
                ("session_key", models.TextField(db_index=True, default=utils.shortcuts.rand_str, unique=True)),
                ("machine_fingerprint", models.TextField()),
                ("locked_ip", models.TextField()),
                ("client_version", models.TextField()),
                ("status", models.TextField(db_index=True, default="active")),
                ("lock_reason", models.TextField(blank=True, null=True)),
                ("last_heartbeat", models.DateTimeField(default=django.utils.timezone.now)),
                ("started_at", models.DateTimeField(auto_now_add=True)),
                ("ended_at", models.DateTimeField(blank=True, null=True)),
                ("unlock_count", models.IntegerField(default=0)),
                ("extra", utils.models.JSONField(default=dict)),
                ("contest", models.ForeignKey(on_delete=django.db.models.deletion.CASCADE, related_name="exam_sessions", to="contest.Contest")),
                ("user", models.ForeignKey(on_delete=django.db.models.deletion.CASCADE, related_name="exam_sessions", to="account.User")),
            ],
            options={"db_table": "exam_session", "ordering": ("-started_at",)},
        ),
        migrations.CreateModel(
            name="ProctorEvent",
            fields=[
                ("id", models.AutoField(auto_created=True, primary_key=True, serialize=False, verbose_name="ID")),
                ("event_type", models.TextField(db_index=True)),
                ("severity", models.TextField(default="info")),
                ("message", models.TextField(blank=True, null=True)),
                ("payload", utils.models.JSONField(default=dict)),
                ("client_time", models.DateTimeField(blank=True, null=True)),
                ("server_time", models.DateTimeField(db_index=True, default=django.utils.timezone.now)),
                ("contest", models.ForeignKey(on_delete=django.db.models.deletion.CASCADE, related_name="proctor_events", to="contest.Contest")),
                ("created_by", models.ForeignKey(blank=True, null=True, on_delete=django.db.models.deletion.SET_NULL, related_name="created_proctor_events", to="account.User")),
                ("session", models.ForeignKey(on_delete=django.db.models.deletion.CASCADE, related_name="events", to="onlinejudge_proctoring.ExamSession")),
                ("user", models.ForeignKey(on_delete=django.db.models.deletion.CASCADE, related_name="proctor_events", to="account.User")),
            ],
            options={"db_table": "proctor_event", "ordering": ("-server_time",)},
        ),
    ]
