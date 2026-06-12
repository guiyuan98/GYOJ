from onlinejudge_proctoring.services import contains_allowed_host, is_version_allowed, normalize_ip, parse_version


def test_parse_version():
    assert parse_version("1.2.3") == (1, 2, 3)
    assert parse_version("1.2") == (1, 2, 0)
    assert parse_version("bad") == (0, 0, 0)


def test_is_version_allowed():
    assert is_version_allowed("0.2.0", "0.1.0")
    assert is_version_allowed("1.0", "1.0.0")
    assert not is_version_allowed("0.0.9", "0.1.0")


def test_normalize_ip():
    assert normalize_ip("127.000.000.001") == "127.000.000.001"
    assert normalize_ip("127.0.0.1, 10.0.0.1") == "127.0.0.1"


def test_contains_allowed_host():
    assert contains_allowed_host("api.example.com", ["example.com"])
    assert contains_allowed_host("example.com", ["example.com"])
    assert not contains_allowed_host("evil-example.com", ["example.com"])

