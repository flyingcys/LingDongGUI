#!/usr/bin/env python3
"""
为 tinyui 下所有 .c 文件中非 static 函数加入标准 doxygen 注释模板。
@param[in] / @return 留空占位，供手工填写。

策略：逐字符扫描文件，找到 '{' 后往回找函数签名。
"""

import re
import os
import sys

GREEN = "\033[32m"
RESET = "\033[0m"


def find_functions(text):
    """Return list of (func_line_idx, func_name, params_list, ret_type, has_doxygen)."""
    functions = []
    i = 0
    n = len(text)

    while i < n:
        ch = text[i]

        # Skip strings
        if ch == '"':
            i += 1
            while i < n and text[i] != '"':
                if text[i] == '\\':
                    i += 1
                i += 1
            i += 1
            continue

        # Skip char literals
        if ch == "'":
            i += 1
            while i < n and text[i] != "'":
                if text[i] == '\\':
                    i += 1
                i += 1
            i += 1
            continue

        # Skip line comments
        if ch == '/' and i + 1 < n and text[i + 1] == '/':
            while i < n and text[i] != '\n':
                i += 1
            i += 1
            continue

        # Skip block comments
        if ch == '/' and i + 1 < n and text[i + 1] == '*':
            i += 2
            while i < n and not (text[i] == '*' and i + 1 < n and text[i + 1] == '/'):
                i += 1
            if i < n:
                i += 2
            continue

        # Skip preprocessor lines
        if ch == '#':
            while i < n and text[i] != '\n':
                i += 1
            i += 1
            continue

        if ch == '\n':
            i += 1
            continue

        # Look for '{'
        if ch == '{':
            func = scan_backwards_for_func(text, i)
            if func:
                functions.append(func)
            i += 1
            continue

        i += 1

    return functions


def find_functions_h(text):
    """Find function DECLARATIONS (prototypes) in .h files.
    Returns list of (func_line_idx, func_name, params_list, ret_type, has_doxygen)."""
    functions = []
    i = 0
    n = len(text)

    while i < n:
        ch = text[i]

        # Skip strings
        if ch == '"':
            i += 1
            while i < n and text[i] != '"':
                if text[i] == '\\':
                    i += 1
                i += 1
            i += 1
            continue
        # Skip char literals
        if ch == "'":
            i += 1
            while i < n and text[i] != "'":
                if text[i] == '\\':
                    i += 1
                i += 1
            i += 1
            continue
        # Skip line comments
        if ch == '/' and i + 1 < n and text[i + 1] == '/':
            while i < n and text[i] != '\n':
                i += 1
            i += 1
            continue
        # Skip block comments
        if ch == '/' and i + 1 < n and text[i + 1] == '*':
            i += 2
            while i < n and not (text[i] == '*' and i + 1 < n and text[i + 1] == '/'):
                i += 1
            if i < n:
                i += 2
            continue
        # Skip preprocessor lines
        if ch == '#':
            while i < n and text[i] != '\n':
                i += 1
            i += 1
            continue
        if ch == '\n':
            i += 1
            continue

        # Look for ';' — potential end of function declaration
        if ch == ';':
            func = scan_backwards_for_decl(text, i)
            if func:
                functions.append(func)
            i += 1
            continue

        i += 1

    return functions


def scan_backwards_for_decl(text, semi_pos):
    """Scan backwards from ';' to find function declaration.
    Returns (func_line_idx, func_name, params_list, ret_type, has_doxygen) or None."""

    # Skip whitespace before ';'
    pos = semi_pos - 1
    while pos >= 0 and text[pos] in ' \t\n\r':
        pos -= 1

    if pos < 0 or text[pos] != ')':
        return None

    # Match ')' with '('
    paren_depth = 1
    pos -= 1
    while pos >= 0 and paren_depth > 0:
        if text[pos] == ')':
            paren_depth += 1
        elif text[pos] == '(':
            paren_depth -= 1
        pos -= 1

    if paren_depth != 0:
        return None

    paren_pos = pos + 1  # position of '('

    # Extract params text
    param_start = paren_pos + 1
    pd = 1
    pend = param_start
    while pend < semi_pos and pd > 0:
        if text[pend] == '(':
            pd += 1
        elif text[pend] == ')':
            pd -= 1
        pend += 1
    param_text = text[param_start:pend - 1]

    # Scan backwards from '(' to find function name
    pos = paren_pos - 1
    while pos >= 0 and text[pos] in ' \t\n\r':
        pos -= 1

    # Read identifier backwards
    func_name_end = pos + 1
    while pos >= 0 and (text[pos].isalnum() or text[pos] == '_'):
        pos -= 1
    func_name_start = pos + 1
    func_name = text[func_name_start:func_name_end]

    if not func_name:
        return None

    # Check keywords
    keywords = {'if', 'for', 'while', 'switch', 'sizeof', 'return', 'catch', 'else', 'do'}
    if func_name in keywords:
        return None
    # Skip sizeof-like patterns
    if func_name in ('uint8_t', 'uint16_t', 'uint32_t', 'int8_t', 'int16_t', 'int32_t', 'size_t'):
        return None

    # Check for '=' before '(' (assignment)
    line_start = text.rfind('\n', 0, paren_pos)
    if line_start == -1:
        line_start = 0
    else:
        line_start += 1
    before_paren = text[line_start:paren_pos]
    bd = 0
    in_s = False
    has_assign = False
    for ch in before_paren:
        if ch == '"':
            in_s = not in_s
        if in_s:
            continue
        if ch == '(':
            bd += 1
        elif ch == ')':
            bd -= 1
        elif ch == '=' and bd == 0:
            has_assign = True
            break
    if has_assign:
        return None

    # Scan backwards before function name to get return type
    pos = func_name_start - 1
    while pos >= 0 and text[pos] in ' \t\n\r':
        pos -= 1

    ret_tokens = []
    while pos >= 0:
        if pos >= 2 and text[pos - 1:pos + 1] == '*/':
            break
        if pos >= 0 and text[pos] == '/' and pos + 1 < len(text) and text[pos + 1] == '*':
            break
        if text[pos] in ' \t\n\r':
            pos -= 1
            continue
        if text[pos] == '*':
            ret_tokens.append('*')
            pos -= 1
            continue
        end = pos + 1
        while pos >= 0 and (text[pos].isalnum() or text[pos] == '_'):
            pos -= 1
        if pos + 1 < end:
            token = text[pos + 1:end]
            if token == 'static':
                return None
            if token in ('struct', 'enum', 'union', 'const', 'extern'):
                ret_tokens.append(token)
            else:
                ret_tokens.append(token)
        else:
            break
        while pos >= 0 and text[pos] in ' \t\n\r':
            pos -= 1
        if pos >= 0 and (text[pos].isalnum() or text[pos] == '_' or text[pos] == '*'):
            continue
        else:
            break

    ret_tokens.reverse()
    ret_type = ' '.join(ret_tokens)
    ret_type = re.sub(r'\* (\*| )', '*', ret_type)

    # Check for doxygen before declaration
    has_doxygen = has_doxygen_before(text, line_start)

    # Get line number
    func_line_idx = text[:func_name_start].count('\n')

    # Extract param names
    params = extract_param_names_h(param_text)

    return (func_line_idx, func_name, params, ret_type, has_doxygen)


def extract_param_names_h(param_text):
    """Extract parameter names from .h declaration params."""
    # Reuse existing extract_param_names function (defined below)
    # Must be called after its definition, so we define it inline
    return _extract_params_inline(param_text)


def _extract_params_inline(param_text):
    """Inline param extraction for .h declarations - simplified version."""
    param_text = param_text.strip()
    if param_text == 'void' or not param_text:
        return []

    type_kw = {
        "int", "char", "float", "double", "long", "short", "unsigned",
        "signed", "const", "volatile", "void", "struct", "enum", "union",
        "auto", "register", "extern", "inline", "restrict",
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

    parts = []
    depth = 0
    cur = ""
    for ch in param_text:
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

    names = []
    for p in parts:
        p = re.sub(r'/\*.*?\*/|//.*', '', p).strip()
        if not p or p == 'void':
            continue
        tokens = re.findall(r'[*]+|[^*\s]+', p)
        merged = []
        for t in tokens:
            if merged and set(t) == {'*'}:
                merged[-1] = merged[-1] + t
            else:
                merged.append(t)
        name = None
        for t in reversed(merged):
            tc = t.rstrip(';[],')
            if tc not in type_kw and not set(tc).issubset({'*'}):
                name = tc
                break
        if name:
            names.append(name)

    return names


def scan_backwards_for_func(text, brace_pos):
    """Scan backwards from '{' to find function signature.
    Returns (func_line_idx, func_name, params, ret_type, has_doxygen) or None."""

    # Skip whitespace before '{'
    pos = brace_pos - 1
    while pos >= 0 and text[pos] in ' \t\n\r':
        pos -= 1

    if pos < 0 or text[pos] != ')':
        return None

    # Match ')' with '('
    paren_depth = 1
    pos -= 1
    while pos >= 0 and paren_depth > 0:
        if text[pos] == ')':
            paren_depth += 1
        elif text[pos] == '(':
            paren_depth -= 1
        pos -= 1

    if paren_depth != 0:
        return None

    # pos is now at the character before '(' (actually one past, since we decremented)
    # Let's fix: after the loop, pos points to char just before '('
    paren_pos = pos + 1  # position of '('

    # Parameter text is between '(' and ')'
    param_start = paren_pos + 1
    param_end = brace_pos - 1
    # Find matching ')' by scanning forward from param_start
    pd = 1
    p_end = param_start
    while p_end <= param_end and pd > 0:
        if text[p_end] == '(':
            pd += 1
        elif text[p_end] == ')':
            pd -= 1
        p_end += 1
    param_text = text[param_start:p_end - 1]  # text between ( and ) but not including them

    # Scan backwards from '(' to find function name
    pos = paren_pos - 1
    while pos >= 0 and text[pos] in ' \t\n\r':
        pos -= 1

    # Read identifier backwards
    func_name_end = pos + 1
    while pos >= 0 and (text[pos].isalnum() or text[pos] == '_'):
        pos -= 1
    func_name_start = pos + 1
    func_name = text[func_name_start:func_name_end]

    if not func_name:
        return None

    # Check keywords
    keywords = {'if', 'for', 'while', 'switch', 'return', 'sizeof',
                'typeof', '__typeof__', 'catch', 'case', 'else', 'do'}
    if func_name in keywords:
        return None

    # Check for '=' before '(' on same logical line
    line_start = text.rfind('\n', 0, paren_pos)
    if line_start == -1:
        line_start = 0
    else:
        line_start += 1
    before_paren = text[line_start:paren_pos]

    # Check for '=' at brace-depth 0
    bd = 0
    in_s = False
    has_assign = False
    for ch in before_paren:
        if ch == '"':
            in_s = not in_s
        if in_s:
            continue
        if ch == '(':
            bd += 1
        elif ch == ')':
            bd -= 1
        elif ch == '=' and bd == 0:
            has_assign = True
            break
    if has_assign:
        return None

    # Scan backwards before function name to get return type
    pos = func_name_start - 1
    while pos >= 0 and text[pos] in ' \t\n\r':
        pos -= 1

    # Collect return type (read backwards)
    ret_tokens = []
    while pos >= 0:
        # Check for block comment boundary
        if pos >= 2 and text[pos - 1:pos + 1] == '*/':
            break
        if pos >= 0 and text[pos] == '/' and pos + 1 < len(text) and text[pos + 1] == '*':
            break

        # Skip whitespace
        if text[pos] in ' \t\n\r':
            pos -= 1
            continue

        # Check for '*' (pointer)
        if text[pos] == '*':
            ret_tokens.append('*')
            pos -= 1
            continue

        # Read identifier backwards
        end = pos + 1
        while pos >= 0 and (text[pos].isalnum() or text[pos] == '_'):
            pos -= 1
        if pos + 1 < end:
            token = text[pos + 1:end]
            if token == 'static':
                return None  # static function, skip
            ret_tokens.append(token)
        else:
            break

        # After reading identifier, check what's before: more type words or break
        while pos >= 0 and text[pos] in ' \t\n\r':
            pos -= 1
        # If there's another type word (not newline-terminated statement), continue
        if pos >= 0 and (text[pos].isalnum() or text[pos] == '_' or text[pos] == '*'):
            continue
        else:
            break

    ret_tokens.reverse()
    ret_type = ' '.join(ret_tokens)
    # Cleanup pointer notation
    ret_type = re.sub(r'\* (\*| )', '*', ret_type)

    # Check for doxygen before function
    has_doxygen = has_doxygen_before(text, line_start)

    # Get function line number
    func_line_idx = text[:func_name_start].count('\n')

    # Extract param names
    params = extract_param_names(param_text)

    return (func_line_idx, func_name, params, ret_type, has_doxygen)


def extract_param_names(param_text):
    """Extract parameter names from text between parentheses."""
    param_text = param_text.strip()
    if param_text == 'void' or not param_text:
        return []

    type_keywords = {
        "int", "char", "float", "double", "long", "short", "unsigned",
        "signed", "const", "volatile", "void", "struct", "enum", "union",
        "auto", "register", "extern", "inline", "restrict",
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

    # Split by comma at depth 0
    parts = []
    depth = 0
    cur = ""
    for ch in param_text:
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

    names = []
    for p in parts:
        p = re.sub(r'/\*.*?\*/|//.*', '', p).strip()
        if not p or p == 'void':
            continue

        tokens = re.findall(r'[*]+|[^*\s]+', p)
        merged = []
        for t in tokens:
            if merged and set(t) == {'*'}:
                merged[-1] = merged[-1] + t
            else:
                merged.append(t)

        # Find param name (last non-type-word)
        name = None
        for t in reversed(merged):
            t_clean = t.rstrip(';[],')
            if t_clean not in type_keywords and not set(t_clean).issubset({'*'}):
                name = t_clean
                break

        if name:
            names.append(name)

    return names


def has_doxygen_before(text, line_start):
    """Check if doxygen block comment ends before line_start position."""
    pos = line_start - 1
    while pos >= 0 and text[pos] in ' \t\n\r':
        pos -= 1
    if pos < 0:
        return False

    # Check for '*/'
    if pos >= 0 and text[pos] == '/':
        if pos - 1 >= 0 and text[pos - 1] == '*':
            pos -= 2
            while pos >= 0 and not (text[pos] == '/' and pos + 1 < len(text) and text[pos + 1] == '*'):
                pos -= 1
            if pos >= 0 and pos + 1 < len(text):
                return text[pos:pos + 2] == '/**'
    return False


def process_file(filepath):
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    is_header = filepath.endswith('.h')
    functions = find_functions_h(content) if is_header else find_functions(content)

    if not functions:
        return False

    # Sort descending by line index
    functions.sort(key=lambda f: f[0], reverse=True)

    lines = content.split('\n')

    for func_ln, func_name, params, ret_type, has_doxygen in functions:
        if has_doxygen:
            continue

        # Build doxygen comment
        doxy = []
        doxy.append('/**')
        doxy.append(' * @brief')
        doxy.append(' *')
        if func_name == 'main':
            doxy.append(' * @return')
        else:
            for p in params:
                doxy.append(' * @param[in] %s' % p)
            if ret_type and ret_type not in ('void', ''):
                doxy.append(' * @return')
        doxy.append(' */')

        # Count blank lines before function
        j = func_ln - 1
        blanks = 0
        while j >= 0 and lines[j].strip() == '':
            blanks += 1
            j -= 1

        insert_at = func_ln - blanks

        # Replace blank lines with: [blank, doxygen..., blank, (original blanks? no)]
        # Actually: remove existing blanks, then insert: blank + doxygen + blank
        if blanks > 0:
            lines[insert_at:insert_at + blanks] = [''] + doxy + ['']
        else:
            lines[insert_at:insert_at] = [''] + doxy + ['']

        relpath = os.path.relpath(filepath)
        param_str = ', '.join(params) if params else 'void'
        print(f"  {GREEN}[ADD]{RESET} {relpath}:{func_ln + 1} {func_name}({param_str})")

    new_content = '\n'.join(lines)
    with open(filepath, 'w', encoding='utf-8') as f:
        f.write(new_content)

    return True


def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)
    os.chdir(project_root)

    all_files = []
    for root_dir, dirs, files in os.walk('tinyui'):
        dirs[:] = [d for d in dirs if not d.startswith('.')]
        for f in files:
            if f.endswith('.c') or f.endswith('.h'):
                all_files.append(os.path.join(root_dir, f))

    all_files.sort()

    added = 0
    skipped = 0

    print("Scanning for non-static functions...\n")

    for fp in all_files:
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
