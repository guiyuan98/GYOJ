import ipaddress
import re
from typing import Iterable, List, Optional, Tuple


SEMVER_RE = re.compile(r"^(\d+)(?:\.(\d+))?(?:\.(\d+))?")


def parse_version(value: str) -> Tuple[int, int, int]:
    match = SEMVER_RE.match(value or "")
    if not match:
        return 0, 0, 0
    major, minor, patch = match.groups()
    return int(major), int(minor or 0), int(patch or 0)


def is_version_allowed(client_version: str, minimum_version: str) -> bool:
    if not minimum_version:
        return True
    return parse_version(client_version) >= parse_version(minimum_version)


def normalize_ip(value: Optional[str]) -> str:
    if not value:
        return ""
    candidate = value.split(",")[0].strip()
    try:
        return str(ipaddress.ip_address(candidate))
    except ValueError:
        return candidate


def client_ip_from_request(request) -> str:
    session_ip = getattr(request, "session", {}).get("ip")
    if session_ip:
        return normalize_ip(session_ip)
    forwarded = request.META.get("HTTP_X_FORWARDED_FOR")
    if forwarded:
        return normalize_ip(forwarded)
    return normalize_ip(request.META.get("REMOTE_ADDR"))


def find_session_violations(policy, session, request_ip: str, machine_fingerprint: str,
                            client_version: str) -> List[str]:
    reasons = []
    if policy.lock_ip and session.locked_ip and normalize_ip(request_ip) != normalize_ip(session.locked_ip):
        reasons.append("ip_changed")
    if machine_fingerprint and session.machine_fingerprint != machine_fingerprint:
        reasons.append("machine_changed")
    if not is_version_allowed(client_version, policy.client_min_version):
        reasons.append("client_version_too_old")
    return reasons


def contains_allowed_host(host: str, allowlist: Iterable[str]) -> bool:
    host = (host or "").lower().strip()
    for item in allowlist or []:
        pattern = str(item).lower().strip()
        if not pattern:
            continue
        if host == pattern or host.endswith("." + pattern):
            return True
    return False

