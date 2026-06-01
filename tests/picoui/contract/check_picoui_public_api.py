from pathlib import Path
import re


ROOT = Path(__file__).resolve().parents[3]
PUBLIC_DIR = ROOT / "picoui" / "include" / "picoui"
ALLOWED_FUNCTION_PREFIX = "picoui_"
ALLOWED_MACRO_PREFIX = "PICOUI_"
ALLOWED_TYPE_PREFIX = "picoui_"

STRUCT_RE = re.compile(r"\bstruct\s+(?P<name>[A-Za-z_][A-Za-z0-9_]*)")
ENUM_RE = re.compile(r"\benum\s+(?P<name>[A-Za-z_][A-Za-z0-9_]*)")
TYPEDEF_CB_RE = re.compile(r"typedef\s+.+\(\*(?P<name>[A-Za-z_][A-Za-z0-9_]*)\)\s*\(")
LD_IDENTIFIER_RE = re.compile(r"\bld[A-Za-z0-9_]*\b")
ARM_IDENTIFIER_RE = re.compile(r"\barm_2d_[A-Za-z0-9_]*\b")
SIGNAL_IDENTIFIER_RE = re.compile(r"\bSIGNAL_[A-Za-z0-9_]*\b")
IDENTIFIER_AT_END_RE = re.compile(r"(?P<name>[A-Za-z_][A-Za-z0-9_]*)\s*$")
OPAQUE_ARM_TYPEDEF_RE = re.compile(
    r"typedef\s+struct\s+arm_2d_[A-Za-z0-9_]*\s+arm_2d_[A-Za-z0-9_]*\s*;"
)


def assert_allowed_prefix(name: str, *, header: Path, kind: str, prefix: str) -> None:
    assert name.startswith(prefix), (
        f"{header.name} exports {kind} '{name}' outside allowed prefix '{prefix}'"
    )


def check_macro_prefixes(header: Path, text: str) -> None:
    for line in text.splitlines():
        stripped = line.strip()
        if not stripped.startswith("#"):
            continue
        parts = stripped.split()
        if len(parts) < 2 or parts[0] not in {"#define", "#ifndef"}:
            continue
        assert_allowed_prefix(parts[1], header=header, kind="macro", prefix=ALLOWED_MACRO_PREFIX)


def check_type_prefixes(header: Path, text: str) -> None:
    for match in STRUCT_RE.finditer(text):
        assert_allowed_prefix(
            match.group("name"), header=header, kind="struct tag", prefix=ALLOWED_TYPE_PREFIX
        )
    for match in ENUM_RE.finditer(text):
        assert_allowed_prefix(
            match.group("name"), header=header, kind="enum tag", prefix=ALLOWED_TYPE_PREFIX
        )
    for match in TYPEDEF_CB_RE.finditer(text):
        assert_allowed_prefix(
            match.group("name"), header=header, kind="typedef callback", prefix=ALLOWED_TYPE_PREFIX
        )


def check_function_prefixes(header: Path, text: str) -> None:
    for statement in text.split(";"):
        normalized = " ".join(statement.split())
        if not normalized or normalized.startswith("#") or normalized.startswith("typedef"):
            continue
        if "(" not in normalized:
            continue
        declaration_head = normalized.split("(", 1)[0].strip()
        if declaration_head.endswith(")") or "(*" in declaration_head:
            continue
        match = IDENTIFIER_AT_END_RE.search(declaration_head)
        assert match is not None, f"{header.name} has unrecognized function declaration: {normalized}"
        assert_allowed_prefix(
            match.group("name"), header=header, kind="function", prefix=ALLOWED_FUNCTION_PREFIX
        )


def check_forbidden_identifiers(header: Path, text: str) -> None:
    sanitized = OPAQUE_ARM_TYPEDEF_RE.sub("", text)
    forbidden_patterns = (
        ("ld*", LD_IDENTIFIER_RE),
        ("arm_2d_*", ARM_IDENTIFIER_RE),
        ("SIGNAL_*", SIGNAL_IDENTIFIER_RE),
    )
    for label, pattern in forbidden_patterns:
        match = pattern.search(sanitized)
        assert match is None, f"{header.name} leaks forbidden identifier '{match.group(0)}' ({label})"


def main() -> int:
    headers = sorted(PUBLIC_DIR.glob("*.h"))
    assert headers, "expected PicoUI public headers to exist"
    for header in headers:
        text = header.read_text(encoding="utf-8")
        check_forbidden_identifiers(header, text)
        check_macro_prefixes(header, text)
        check_type_prefixes(header, text)
        check_function_prefixes(header, text)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
