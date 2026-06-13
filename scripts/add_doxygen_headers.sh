#!/usr/bin/env bash
# 为 tinyui 下所有 .c 文件中非 static 函数加入标准 doxygen 注释模板
# @param[in]/@param[out]/@return 留空，供手工填写
set -euo pipefail

cd "$(git rev-parse --show-toplevel)"

python3 -c '
import re
import os
import sys

def is_doxygen_comment(lines, idx):
    """Check if line idx is end of doxygen comment block before function"""
    # Go backwards skipping blank lines
    i = idx - 1
    while i >= 0 and lines[i].strip() == "":
        i -= 1
    if i < 0:
        return False
    # Last line of a doxygen block: " */"
    if lines[i].strip() == "*/":
        # Find the opening "/**"
        j = i - 1
        while j >= 0:
            if "/**" in lines[j]:
                return True
            if lines[j].strip().startswith("/*") and "*/" not in lines[j]:
                return False  # Regular /* comment, not doxygen
            j -= 1
    return False

def extract_params_and_return(func_block):
    """Extract parameter names and return type from function definition block"""
    text = " ".join(func_block)
    params = []
    ret_type = "void"

    # Extract return type (everything before function name)
    # Match: [return_type] function_name(...)
    # function name is last identifier before "("
    match = re.search(r"(\w+)\s*\(", text)
    if not match:
        return [], "void"

    # Find the function name position
    paren_pos = text.index("(")
    before_paren = text[:paren_pos].strip()

    # Try to extract function name - last word before "("
    words = before_paren.split()
    func_name = words[-1] if words else ""

    # Everything before the function name (excluding last word) is return type
    # Remove pointer asterisks, etc
    if len(words) > 1:
        ret_words = words[:-1]
        ret_type = " ".join(ret_words).strip()
        # Clean up "void *" etc
        ret_type = re.sub(r"\s*\*\s*", "* ", ret_type).strip()

    # Extract params from between parentheses
    paren_depth = 0
    paren_start = None
    param_texts = []
    for i, ch in enumerate(text):
        if ch == "(":
            if paren_depth == 0:
                paren_start = i
            paren_depth += 1
        elif ch == ")":
            paren_depth -= 1
            if paren_depth == 0 and paren_start is not None:
                param_texts.append(text[paren_start+1:i])
                paren_start = None
    if not param_texts:
        return [], ret_type

    param_text = param_texts[0]
    # Handle void
    if param_text.strip() == "void":
        return [], ret_type

    # Split by commas, but not inside nested parens (function pointers)
    params_raw = []
    depth = 0
    current = ""
    for ch in param_text:
        if ch in "({[":
            depth += 1
            current += ch
        elif ch in ")}]":
            depth -= 1
            current += ch
        elif ch == "," and depth == 0:
            params_raw.append(current.strip())
            current = ""
        else:
            current += ch
    if current.strip():
        params_raw.append(current.strip())

    for p in params_raw:
        # Extract the last word as parameter name
        p = re.sub(r"/\*.*?\*/|//.*", "", p).strip()
        if not p:
            continue
        # Skip if it looks like a type-only (e.g., "void")
        if p == "void":
            continue
        # If it has a struct/enum prefix and no name, skip
        words = p.replace("*", " * ").split()
        words = [w for w in words if w.strip()]
        if not words:
            continue
        # The actual param name is typically the last word that isnt a type keyword
        type_keywords = {"int", "char", "float", "double", "long", "short", "unsigned",
                        "signed", "const", "volatile", "void", "struct", "enum", "union",
                        "uint8_t", "uint16_t", "uint32_t", "uint64_t", "int8_t", "int16_t",
                        "int32_t", "int64_t", "size_t", "ssize_t", "bool", "intptr_t",
                        "uintptr_t", "uint_fast8_t", "uint_fast16_t", "uint_fast32_t",
                        "uint_least8_t"}
        name_candidates = [w for w in words if w not in type_keywords and not w.startswith("*")]
        if name_candidates:
            # If there are pointer stars, param name might be after them
            param_name = name_candidates[-1]
            # Clean trailing punctuation
            param_name = param_name.rstrip(";[],")
            params.append(param_name)

    return params, ret_type

def process_file(filepath):
    with open(filepath, "r", encoding="utf-8") as f:
        content = f.read()
        lines = content.split("\n")

    # Handle line endings - work with list
    modified = False
    new_lines = []
    i = 0

    while i < len(lines):
        line = lines[i]

        # Check if this looks like start of a non-static function definition
        # Function pattern: return_type_and_name + args + opening brace
        # We look for lines that could be "return_type func_name(" at start
        # or continuation of multi-line return type

        stripped = line.lstrip()
        # Must not be comment, preprocessor, or inside block
        first_char = stripped[0] if stripped else ""

        # Skip: comments, preprocessor, static, empty, control flow
        if stripped.startswith("//") or stripped.startswith("/*") or stripped.startswith("*"):
            new_lines.append(line)
            i += 1
            continue
        if stripped.startswith("#"):
            new_lines.append(line)
            i += 1
            continue
        if not stripped:
            new_lines.append(line)
            i += 1
            continue

        # Detect: "static" keyword at start of meaningful line
        if re.match(r"^\s*static\s", line):
            new_lines.append(line)
            i += 1
            continue

        # Counter for inline assembly, string literals etc
        if re.match(r"^\s*(if|else|for|while|do|switch|case|return|break|continue|typedef|struct|enum|union)\b", stripped):
            new_lines.append(line)
            i += 1
            continue

        # Try to collect a function definition block
        # Start looking from current line, accumulate until we find "{"
        # that isnt preceded by ";" or "=" (forward declaration or initializer)
        j = i
        func_lines = []
        found_paren = False
        found_brace = False
        while j < len(lines):
            l = lines[j]
            # Stop if we hit a comment at column 0 between lines
            if j > i and (l.strip().startswith("//") or l.strip().startswith("/*")):
                break
            # Check for "(" - start of parameter list
            if "(" in l and not found_paren:
                # Exclude control flow
                if re.match(r"^\s*(if|else|for|while|switch|catch)\b", l.lstrip()):
                    break
                found_paren = True

            # Check for "{" at brace depth 0 outside parens
            brace_depth = 0
            paren_depth = 0
            in_string = False
            hit_brace = False
            for ch in l:
                if ch == '"' and not in_string:
                    in_string = True
                elif ch == '"' and in_string:
                    in_string = False
                if in_string:
                    continue
                if ch == "(":
                    paren_depth += 1
                elif ch == ")":
                    paren_depth -= 1
                if paren_depth == 0:
                    if ch == "{":
                        hit_brace = True
                    elif ch == ";" and found_paren:
                        # ";" before "{" means forward declaration or inline struct
                        hit_brace = False
                        break
            if hit_brace:
                found_brace = True
                if found_paren:
                    func_lines.append(l)
                    break
                else:
                    # "{ without "(" - not a function
                    break
            if ";" in l and found_paren:
                # ";" before "{": forward decl or struct definition
                # But only break if ";" at top level
                semi_at_top = False
                pd = 0
                for ch in l:
                    if ch == "(":
                        pd += 1
                    elif ch == ")":
                        pd -= 1
                    elif ch == ";" and pd == 0:
                        semi_at_top = True
                        break
                if semi_at_top:
                    break
            func_lines.append(l)
            j += 1

        if not found_brace or not found_paren:
            # Not a function definition
            new_lines.append(line)
            i += 1
            continue

        # Check existing doxygen comment before function
        # Check line before i (skip blanks)
        has_doxygen = False
        k = i - 1
        while k >= 0 and lines[k].strip() == "":
            k -= 1
        if k >= 0 and lines[k].strip() == "*/":
            k2 = k - 1
            while k2 >= 0:
                if "/**" in lines[k2]:
                    has_doxygen = True
                    break
                if lines[k2].strip().startswith("/*"):
                    break
                k2 -= 1

        if has_doxygen:
            # Already has doxygen, copy as-is
            new_lines.append(line)
            i += 1
            continue

        # This is a non-static function without doxygen!
        # Extract function info
        func_sig = "\n".join(func_lines)

        # Get function name for @brief
        name_match = re.search(r"(\w+)\s*\(", func_sig)
        func_name = name_match.group(1) if name_match else "unknown"

        # Get params
        params, ret_type = extract_params_and_return(func_lines)

        # Build doxygen comment
        doxygen_lines = []
        doxygen_lines.append("/**")
        doxygen_lines.append(" * @brief %s" % func_name)
        doxygen_lines.append(" *")
        for p in params:
            doxygen_lines.append(" * @param[in] %s" % p)
        if ret_type not in ("void", ""):
            doxygen_lines.append(" * @return")
        doxygen_lines.append(" */")

        # Prepend doxygen + blank line if needed
        # Preserve blank lines before function (skip trailing whitespace)
        trailing_blanks = []
        while trailing_blanks and trailing_blanks[-1].strip() == "":
            pass  # already handled

        # Add doxygen before the function, keeping existing blank lines above it
        insert_idx = i
        # Count blank lines before function
        blanks_before = 0
        k = i - 1
        while k >= 0 and lines[k].strip() == "":
            blanks_before += 1
            k -= 1

        # Keep one blank line between doxygen and function
        # Add: blank line before doxygen (to separate from prior code)
        if blanks_before > 0:
            # Keep existing blank lines structure, insert doxygen after them
            # Actually, simpler: keep all blanks before, insert doxygen just before func
            pass

        # Emit lines up to i
        for idx in range(i, len(func_lines)):
            # Actually, weve already emitted up to i in new_lines, need to redo
            pass

        # Let me redo - emit all existing lines before function, then doxygen, then function

    return modified, content
'

echo "Script infrastructure created - writing final version..."

# Write the actual Python script
python3 << 'PYEOF'
import re
import os
import sys

# ANSI colors
GREEN = "\033[32m"
SKIP = "\033[2m\033[37m"
RESET = "\033[0m"

def extract_params(func_text):
    """Extract parameter names from function signature text"""
    # Find parameter list between outermost parens
    depth = 0
    start = None
    for i, ch in enumerate(func_text):
        if ch == '(':
            if depth == 0:
                start = i
            depth += 1
        elif ch == ')':
            depth -= 1
            if depth == 0 and start is not None:
                params_str = func_text[start+1:i]
                break
    else:
        return []

    params_str = params_str.strip()
    if params_str == "void" or not params_str:
        return []

    # Split by comma at depth 0 (handle nested parens for function pointers)
    parts = []
    depth = 0
    cur = ""
    for ch in params_str:
        if ch in '({[':
            depth += 1
            cur += ch
        elif ch in ')}]':
            depth -= 1
            cur += ch
        elif ch == ',' and depth == 0:
            parts.append(cur.strip())
            cur = ""
        else:
            cur += ch
    if cur.strip():
        parts.append(cur.strip())

    # Extract parameter names
    # Remove comments and clean
    type_keywords = {
        "int", "char", "float", "double", "long", "short", "unsigned",
        "signed", "const", "volatile", "void", "struct", "enum", "union",
        "uint8_t", "uint16_t", "uint32_t", "uint64_t", "int8_t", "int16_t",
        "int32_t", "int64_t", "size_t", "ssize_t", "bool", "intptr_t",
        "uintptr_t", "uint_fast8_t", "uint_fast16_t", "uint_fast32_t",
        "uint_least8_t", "uint_least16_t", "uint_least32_t",
        "int_least8_t", "int_least16_t", "int_least32_t", "int_least64_t",
        "uint_least64_t", "uintmax_t", "intmax_t", "int_fast8_t",
        "int_fast16_t", "int_fast32_t", "int_fast64_t", "uint_fast64_t",
        "ptrdiff_t", "wchar_t", "time_t", "FILE", "va_list",
        "ldColor", "ldTexture",
    }

    names = []
    for p in parts:
        # Remove comments
        p = re.sub(r'/\*.*?\*/|//.*', '', p).strip()
        if not p:
            continue
        # Handle unnamed params (just type)
        if re.match(r'^\s*(void|struct\s+\w+|enum\s+\w+)\s*$', p):
            continue

        # Split by '*' to separate type from name
        # Strategy: last "word" that isnt a type keyword
        # Keep pointer asterisks attached
        tokens = re.findall(r'[*]+|[^*\s]+', p)
        # Join adjacent * to following word
        cleaned = []
        for t in tokens:
            if cleaned and set(t) == {'*'}:
                cleaned[-1] = cleaned[-1] + t
            else:
                cleaned.append(t)

        # Filter out type keywords from end
        name = None
        for t in reversed(cleaned):
            if t not in type_keywords and not set(t).issubset({'*'}):
                name = t.rstrip(';[],')
                break

        if name:
            names.append(name)

    return names


def extract_return_type_and_name(file_text, func_start_line):
    """Scan backwards from function body `{` to find return type and function name.
    Returns (return_type, func_name)"""
    lines = file_text.split('\n')
    # Start from func_start_line, go backwards including current line
    # Collect lines until we hit a semicolon, open brace at depth 0, or blank line
    sig_lines = []

    # But easier: from the function's first line, go forward past return type
    # Let me just find the function name in the signature
    return None


def collect_func_sig(filepath, line_idx, lines):
    """Starting from line_idx, collect whole function signature until '{'.
    Returns (sig_text, end_line_idx) or None if not a function."""
    # Collect lines until we find '{' at paren-depth 0
    sig_parts = []
    j = line_idx
    found_open_paren = False
    while j < len(lines):
        l = lines[j]

        # Stop if we hit certain patterns at column 0
        stripped = l.lstrip()
        if j > line_idx:
            if not stripped or stripped.startswith('#') or stripped.startswith('//') or stripped.startswith('/*') or stripped.startswith('*'):
                break

        # Track paren depth to find `{` at top level
        pd = 0
        in_str = False
        for ch in l:
            if ch == '"' and not in_str:
                in_str = True
            elif ch == '"' and in_str:
                in_str = False
            if in_str:
                continue
            if ch == '(':
                found_open_paren = True
                pd += 1
            elif ch == ')':
                pd -= 1
            elif ch == '{' and pd == 0 and found_open_paren:
                # Found it - we have complete signature
                sig_parts.append(l)
                return '\n'.join(sig_parts), j
            elif ch == ';' and pd == 0 and found_open_paren:
                # Forward declaration / struct method decl, not a definition
                return None, j

        sig_parts.append(l)
        j += 1

    return None, j


def process_file(filepath):
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()
        lines = content.split('\n')

    modified = False
    result = []
    i = 0

    while i < len(lines):
        line = lines[i]
        stripped = line.strip()

        # Skip non-candidate lines quickly
        is_candidate = (
            stripped and
            not stripped.startswith('//') and
            not stripped.startswith('/*') and
            not stripped.startswith('*') and
            not stripped.startswith('#') and
            not re.match(r'^\s*(if|else|for|while|do|switch|case|return|break|continue|typedef)\b', stripped)
        )

        if not is_candidate:
            result.append(line)
            i += 1
            continue

        # Check this line could start a new function definition
        # Must not start with static
        if re.match(r'^\s*static\s', line):
            result.append(line)
            i += 1
            continue

        # Try to collect function signature
        sig = collect_func_sig(filepath, i, lines)
        if sig is None or sig[0] is None:
            result.append(line)
            i += 1
            continue

        sig_text, brace_line = sig

        # Check: is there already a doxygen comment before it?
        has_doxygen = False
        k = i - 1
        while k >= 0 and lines[k].strip() == '':
            k -= 1
        if k >= 0 and lines[k].strip() == '*/':
            k2 = k - 1
            while k2 >= 0:
                if '/**' in lines[k2]:
                    has_doxygen = True
                    break
                if lines[k2].strip().startswith('/*'):
                    break
                k2 -= 1

        if has_doxygen:
            # Already documented
            for idx in range(i, brace_line + 1):
                result.append(lines[idx])
            i = brace_line + 1
            continue

        # Extract function name
        name_match = re.search(r'(\w+)\s*\(', sig_text)
        if not name_match:
            for idx in range(i, brace_line + 1):
                result.append(lines[idx])
            i = brace_line + 1
            continue

        func_name = name_match.group(1)

        # Extract params
        params = extract_params(sig_text)

        # Determine return type
        # Everything before function name
        paren_pos = sig_text.index('(')
        before = sig_text[:paren_pos].strip()
        # Last word is function name
        words = before.split()
        ret_type = 'void'
        if len(words) > 1:
            ret_words = words[:-1]
            ret_type = ' '.join(ret_words).strip()
            # Clean pointer spacing
            ret_type = re.sub(r'\s*\*\s*', '* ', ret_type).strip()
        if ret_type.endswith('*'):
            ret_type = ret_type + ' '

        # Build doxygen comment
        doxy = []
        doxy.append('/**')
        doxy.append(' * @brief')
        doxy.append(' *')
        if func_name in ('main',):
            doxy.append(' * @return')
        else:
            if params:
                for p in params:
                    doxy.append(' * @param[in] %s' % p)
            if ret_type not in ('void', ''):
                doxy.append(' * @return')
        doxy.append(' */')

        # Check blank lines before function
        blanks_before = 0
        k = i - 1
        while k >= 0 and lines[k].strip() == '':
            blanks_before += 1
            k -= 1

        # Emit: all lines before i (already in result), then blanks, then doxy, then rest
        # Remove trailing blank lines from result
        while result and result[-1].strip() == '':
            result.pop()

        # Add one blank line above doxygen
        result.append('')
        # Add doxygen
        result.extend(doxy)
        # Add blank line between doxygen and function
        result.append('')

        # Emit function lines
        for idx in range(i, brace_line + 1):
            result.append(lines[idx])

        print(f"  {GREEN}[ADD]{RESET} {filepath}:{i+1} {func_name}({', '.join(params)}) -> {ret_type}")

        i = brace_line + 1
        modified = True

    if modified:
        new_content = '\n'.join(result)
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(new_content)

    return modified


def main():
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    os.chdir(root)

    c_files = []
    for root_dir, dirs, files in os.walk('tinyui'):
        for f in files:
            if f.endswith('.c'):
                c_files.append(os.path.join(root_dir, f))

    c_files.sort()

    added = 0
    skipped = 0

    for fp in c_files:
        try:
            modified = process_file(fp)
            if modified:
                added += 1
            else:
                skipped += 1
        except Exception as e:
            print(f"  {RESET}[ERR]{RESET} {fp}: {e}", file=sys.stderr)

    print(f"\nDone. {added} file(s) updated, {skipped} file(s) unchanged.")


if __name__ == '__main__':
    main()
PYEOF
